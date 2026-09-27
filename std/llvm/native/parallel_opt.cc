/*
 * Salam Programming Language (2024-2026)
 *
 *   +-------------------+
 *   |     S A L A M     |
 *   +-------------------+
 *
 * Designed by Seyyed Ali Mohammadiyeh and the Salam Team
 * Born from a decade of language design experience (since 2018)
 *
 * Repository: https://github.com/SalamLang/Salam
 *
 */

/* Optimizes and code-generates one module on several threads.
 *
 * The module is cut into contiguous partitions of similar size. Each
 * partition keeps the functions it owns and receives available_externally
 * copies of the small functions (and constant globals) its code reaches, so
 * the inliner still sees across partition borders, the way ThinLTO imports
 * functions. Every partition then goes through a bitcode round trip into its
 * own LLVMContext and is optimized and turned into an object file on its own
 * thread. A local symbol that another partition refers to or copies becomes a
 * hidden external named "__salam_p.<name>", so it cannot clash with a library
 * symbol and still stays private to the final executable; every other local
 * symbol stays internal, so it can still be inlined away or deleted. */

#include "parallel_opt.h"

#include "llvm-c/Core.h"
#include "llvm-c/Error.h"
#include "llvm-c/Target.h"
#include "llvm-c/TargetMachine.h"
#include "llvm-c/Transforms/PassBuilder.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace llvm;

namespace
{

const unsigned kImportMaxInstructions = 60;
const unsigned kMinPartitionInstructions = 20000;

unsigned instruction_count(const Function &F)
{
    unsigned n = 0;
    for (const BasicBlock &BB : F)
        n += BB.size();
    return n;
}

bool supported(const Module &M)
{
    if (!M.alias_empty() || !M.ifunc_empty()) return false;
    if (M.getNamedMetadata("llvm.dbg.cu")) return false;
    for (const GlobalObject &GO : M.global_objects())
        if (GO.hasComdat()) return false;
    return true;
}

void externalize(const std::unordered_set<GlobalValue *> &need)
{
    for (GlobalValue *GV : need) {
        if (GV->isDeclaration() || GV->hasAppendingLinkage()) continue;
        if (GV->hasLocalLinkage() || GV->hasLinkOnceLinkage() ||
            GV->hasWeakODRLinkage()) {
            if (GV->hasLocalLinkage()) GV->setName("__salam_p." + GV->getName());
            GV->setLinkage(GlobalValue::ExternalLinkage);
            GV->setVisibility(GlobalValue::HiddenVisibility);
            GV->setDSOLocal(true);
        }
    }
}

void collect_refs(const Constant *C, std::vector<const GlobalValue *> &out,
                  std::unordered_set<const Constant *> &seen)
{
    if (!seen.insert(C).second) return;
    if (const GlobalValue *GV = dyn_cast<GlobalValue>(C)) {
        out.push_back(GV);
        return;
    }
    for (const Use &U : C->operands())
        if (const Constant *Op = dyn_cast<Constant>(U.get())) collect_refs(Op, out, seen);
}

std::vector<const GlobalValue *> refs_of(const Function &F)
{
    std::vector<const GlobalValue *> out;
    std::unordered_set<const Constant *> seen;
    for (const BasicBlock &BB : F)
        for (const Instruction &I : BB)
            for (const Use &U : I.operands())
                if (const Constant *C = dyn_cast<Constant>(U.get()))
                    collect_refs(C, out, seen);
    return out;
}

std::vector<const GlobalValue *> refs_of(const GlobalVariable &G)
{
    std::vector<const GlobalValue *> out;
    std::unordered_set<const Constant *> seen;
    if (G.hasInitializer()) collect_refs(G.getInitializer(), out, seen);
    return out;
}

bool importable(const GlobalValue *GV,
                const std::unordered_map<const Function *, unsigned> &size)
{
    if (GV->isDeclaration() || GV->isInterposable()) return false;
    if (const Function *F = dyn_cast<Function>(GV)) {
        auto it = size.find(F);
        return it != size.end() && it->second <= kImportMaxInstructions &&
               !F->hasFnAttribute(Attribute::NoInline);
    }
    if (const GlobalVariable *G = dyn_cast<GlobalVariable>(GV))
        return G->isConstant() && G->hasInitializer() && !G->hasAppendingLinkage();
    return false;
}

std::string take_error(LLVMErrorRef err)
{
    char *msg = LLVMGetErrorMessage(err);
    std::string s = msg ? msg : "(unknown)";
    if (msg) LLVMDisposeErrorMessage(msg);
    return s;
}

char *dup_c(const std::string &s)
{
    char *p = static_cast<char *>(std::malloc(s.size() + 1));
    if (p) std::memcpy(p, s.c_str(), s.size() + 1);
    return p;
}

} // namespace

extern "C" int salam_parallel_partitions(LLVMModuleRef mod_ref, int jobs)
{
    Module &M = *unwrap(mod_ref);
    if (jobs < 2 || !supported(M)) return 1;
    unsigned total = 0;
    for (const Function &F : M)
        if (!F.isDeclaration()) total += instruction_count(F);
    unsigned min_part = kMinPartitionInstructions;
    if (const char *env = std::getenv("SALAM_LLVM_PARTITION_MIN")) {
        long v = std::strtol(env, nullptr, 10);
        if (v > 0) min_part = static_cast<unsigned>(v);
    }
    unsigned parts = total / min_part;
    if (parts > static_cast<unsigned>(jobs)) parts = static_cast<unsigned>(jobs);
    return parts < 2 ? 1 : static_cast<int>(parts);
}

extern "C" int salam_parallel_opt_emit(LLVMModuleRef mod_ref, LLVMTargetMachineRef tm_ref,
                                       const char *pipeline, int parts,
                                       const char *out_prefix, char **err_out)
{
    if (err_out) *err_out = nullptr;
    Module &M = *unwrap(mod_ref);
    TargetMachine *TM = reinterpret_cast<TargetMachine *>(tm_ref);
    if (parts < 2) {
        if (err_out)
            *err_out = dup_c("parallel optimization needs at least two partitions");
        return 1;
    }

    std::unordered_map<const Function *, unsigned> size;
    std::vector<const Function *> order;
    unsigned total = 0;
    for (const Function &F : M) {
        if (F.isDeclaration()) continue;
        unsigned n = instruction_count(F) + 1;
        size[&F] = n;
        order.push_back(&F);
        total += n;
    }

    std::unordered_map<const GlobalValue *, unsigned> owner;
    {
        unsigned acc = 0;
        for (const Function *F : order) {
            unsigned p = static_cast<unsigned>(
                (static_cast<unsigned long long>(acc) * parts) / total);
            owner[F] = p < static_cast<unsigned>(parts) ? p : parts - 1;
            acc += size[F];
        }
        for (const GlobalVariable &G : M.globals())
            if (!G.isDeclaration()) owner[&G] = 0;
    }

    std::unordered_map<const GlobalValue *, std::vector<const GlobalValue *>> refs_cache;
    auto refs = [&](const GlobalValue *GV) -> const std::vector<const GlobalValue *> & {
        auto it = refs_cache.find(GV);
        if (it != refs_cache.end()) return it->second;
        std::vector<const GlobalValue *> r;
        if (const Function *F = dyn_cast<Function>(GV))
            r = refs_of(*F);
        else if (const GlobalVariable *G = dyn_cast<GlobalVariable>(GV))
            r = refs_of(*G);
        return refs_cache.emplace(GV, std::move(r)).first->second;
    };

    std::vector<std::unordered_set<const GlobalValue *>> keep(parts);
    std::vector<std::unordered_set<const GlobalValue *>> imported(parts);
    for (int p = 0; p < parts; ++p) {
        std::vector<const GlobalValue *> work;
        for (const auto &kv : owner)
            if (kv.second == static_cast<unsigned>(p)) {
                keep[p].insert(kv.first);
                work.push_back(kv.first);
            }
        while (!work.empty()) {
            const GlobalValue *GV = work.back();
            work.pop_back();
            for (const GlobalValue *R : refs(GV)) {
                if (keep[p].count(R) || !importable(R, size)) continue;
                keep[p].insert(R);
                imported[p].insert(R);
                work.push_back(R);
            }
        }
    }

    std::unordered_set<GlobalValue *> need;
    for (int p = 0; p < parts; ++p) {
        for (const GlobalValue *GV : imported[p])
            need.insert(const_cast<GlobalValue *>(GV));
        for (const GlobalValue *GV : keep[p])
            for (const GlobalValue *R : refs(GV)) {
                auto it = owner.find(R);
                if (it != owner.end() && it->second != static_cast<unsigned>(p) &&
                    !imported[p].count(R))
                    need.insert(const_cast<GlobalValue *>(R));
            }
    }
    externalize(need);

    std::vector<SmallVector<char, 0>> bitcode(parts);
    for (int p = 0; p < parts; ++p) {
        ValueToValueMapTy VMap;
        std::unique_ptr<Module> part = CloneModule(
            M, VMap, [&](const GlobalValue *GV) { return keep[p].count(GV) != 0; });
        for (const GlobalValue *GV : imported[p]) {
            GlobalValue *NG = cast<GlobalValue>(VMap[GV]);
            NG->setLinkage(GlobalValue::AvailableExternallyLinkage);
            NG->setVisibility(GlobalValue::DefaultVisibility);
        }
        if (p != 0) {
            std::vector<GlobalVariable *> appending;
            for (GlobalVariable &G : part->globals())
                if (G.hasAppendingLinkage()) appending.push_back(&G);
            for (GlobalVariable *G : appending)
                G->eraseFromParent();
        }
        raw_svector_ostream os(bitcode[p]);
        WriteBitcodeToFile(*part, os);
    }

    const Target &T = TM->getTarget();
    Triple triple = TM->getTargetTriple();
    std::string cpu = TM->getTargetCPU().str();
    std::string features = TM->getTargetFeatureString().str();
    TargetOptions options = TM->Options;
    Reloc::Model reloc = TM->getRelocationModel();
    CodeModel::Model code_model = TM->getCodeModel();
    CodeGenOptLevel level = TM->getOptLevel();
    std::string pipe = pipeline ? pipeline : "";
    std::string prefix = out_prefix ? out_prefix : "salam";

    std::mutex err_mu;
    std::string first_err;
    auto fail = [&](const std::string &msg) {
        std::lock_guard<std::mutex> lock(err_mu);
        if (first_err.empty()) first_err = msg;
    };

    std::vector<std::thread> threads;
    for (int p = 0; p < parts; ++p) {
        threads.emplace_back([&, p]() {
            LLVMContext ctx;
            MemoryBufferRef ref(StringRef(bitcode[p].data(), bitcode[p].size()),
                                "partition");
            Expected<std::unique_ptr<Module>> parsed = parseBitcodeFile(ref, ctx);
            if (!parsed) {
                fail("partition " + std::to_string(p) + ": " +
                     toString(parsed.takeError()));
                return;
            }
            std::unique_ptr<Module> part = std::move(*parsed);
#if LLVM_VERSION_MAJOR >= 21
            std::unique_ptr<TargetMachine> tm(T.createTargetMachine(
                triple, cpu, features, options, reloc, code_model, level));
#else
            std::unique_ptr<TargetMachine> tm(T.createTargetMachine(
                triple.str(), cpu, features, options, reloc, code_model, level));
#endif
            if (!tm) {
                fail("cannot create a target machine for a partition");
                return;
            }
            LLVMTargetMachineRef tmr = reinterpret_cast<LLVMTargetMachineRef>(tm.get());
            if (!pipe.empty()) {
                LLVMPassBuilderOptionsRef pbo = LLVMCreatePassBuilderOptions();
                LLVMErrorRef e = LLVMRunPasses(wrap(part.get()), pipe.c_str(), tmr, pbo);
                LLVMDisposePassBuilderOptions(pbo);
                if (e) {
                    fail("LLVM optimization failed: " + take_error(e));
                    return;
                }
            }
            std::string path = prefix + "." + std::to_string(p) + ".o";
            char *emit_err = nullptr;
            if (LLVMTargetMachineEmitToFile(tmr, wrap(part.get()), path.c_str(),
                                            LLVMObjectFile, &emit_err)) {
                fail(emit_err ? emit_err : "object emission failed");
                if (emit_err) LLVMDisposeMessage(emit_err);
            }
        });
    }
    for (std::thread &t : threads)
        t.join();

    if (!first_err.empty()) {
        if (err_out) *err_out = dup_c(first_err);
        return 1;
    }
    return 0;
}
