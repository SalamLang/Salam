---
title: Running AI workloads with Salam: loading GGUF language models, running ONNX and building ML tools without Python
seo_title: Run LLMs, GGUF and ONNX Models in Salam | Salam Magazine
description: Can you run an LLM or an ONNX model from a native Salam program? What the gguf, llm, onnx and tensor packages do today, their limits, and where Salam fits.
keywords: run LLM locally, gguf, onnx inference, AI inference without Python, run language model CPU, Salam AI packages, llama.cpp GGUF format, tensor library, ONNX runtime alternative, Salam machine learning
category: future
summary: Salam's standard library can read GGUF language models, run small Llama-family models on the processor and execute ONNX graphs, all in pure Salam. Here is what works, what it costs and what it is not meant for.
date: 2026-11-11
---

Yes, you can run AI models from a Salam program, within clear limits. Salam's standard library has a GGUF reader (`ai.gguf`), a small Llama-family inference engine (`ai.llm`), an ONNX graph runner (`ai.onnx`) and the tensor, autograd and neural network packages underneath them. **AI inference in Salam means loading a trained model file and computing its outputs inside a native executable, with no Python interpreter and no outside runtime library.** It runs on the processor only, it is sized for small models, and it is not a replacement for llama.cpp, ONNX Runtime or PyTorch on a GPU.

This article is in the "future" category because the interesting question is where this goes. But the future has to start from the present, so most of what follows is what the repository contains today, with real programs and the limits the package documentation states.

## What "AI workloads" means here {#scope}

People mean several different jobs when they say "run AI". It helps to separate them, because Salam covers some well, some partly and some not at all.

| Job | Typical tool elsewhere | Salam today |
|---|---|---|
| Train a large model on GPUs | PyTorch, JAX | Not covered: no GPU code in the library |
| Serve a large model to many users | vLLM, llama.cpp server, hosted APIs | Call such a server over HTTP |
| Run a small language model inside a program | llama.cpp | `ai.llm` with a GGUF file under 2 GB |
| Run an exported classifier or embedding model | ONNX Runtime | `ai.onnx`, about 65 operators |
| Train a small network on the CPU | PyTorch on CPU | `ai.nn`, `ai.autograd`, `ai.optim` |
| Classic machine learning on tables | scikit-learn | `ai.ml` |
| Exchange arrays with Python | NumPy files | `npy` reads and writes `.npy` and `.npz` |

The pattern is clear: Salam is useful at the small and embedded end, where a program needs a model as one component and you want a single native binary with no Python environment beside it. At the large end, the honest answer is to call a server.

## Reading GGUF model files with ai.gguf {#gguf}

GGUF is the file format of llama.cpp. One file holds the metadata (architecture, sizes, tokenizer vocabulary, chat template) and every weight, usually quantized. The [`ai.gguf` package](/std/ai/gguf/) parses that file: `LoadGguf` reads the header and the tensor list, `meta_int` and `meta_str` read metadata keys, and `LoadTensorF32` expands one weight into a tensor of 32-bit floats.

The program below inspects a model file passed on the command line. It is type-checked here rather than run, because it needs a downloaded model.

```salam
import ai.gguf
import os

func main:
    args := os.Args()
    if args.len() < 2:
        println "usage: inspect model.gguf"
        ret 1
    end
    mut g := gguf.Gguf {}
    try:
        g = gguf.LoadGguf(args.get(1))
    catch e:
        println "not a readable GGUF file:", e.to_str()
        ret 1
    end
    arch := g.meta_str("general.architecture")
    println "model:", g.meta_str("general.name"), "(" + arch + ")"
    println "layers:", g.meta_int(arch + ".block_count", 0)
    println "context:", g.meta_int(arch + ".context_length", 0)
    mut unreadable := 0
    repeat g.tensors.len() in i:
        if gguf.TensorBytes(g, i) < 0:
            unreadable++
        end
    end
    println g.tensors.len(), "tensors,", unreadable, "in a weight type this package cannot expand"
    g.free()
end
```

The last count matters because of the package's main limit. It reads five weight types: `F32`, `F16`, `Q8_0`, `Q4_0` and `Q4_1`. The K-quants that most downloads use, such as `Q4_K_M`, and BF16 are not supported; `TensorBytes` returns -1 for them. `LoadGguf` also reads the whole file into memory first, and a buffer holds at most 2 GB. In practice that means picking the `Q8_0` or `Q4_0` file of a small model such as Qwen2.5 0.5B or TinyLlama 1.1B.

## Running a small language model with ai.llm {#llm}

The [`ai.llm` package](/std/ai/llm/) builds on the GGUF reader and runs Llama-family models on the CPU, in pure Salam. It implements the Llama layer layout (RMSNorm, rotary positions, grouped-query attention, a SwiGLU feed-forward block) plus the attention biases Qwen2 uses. Its documentation lists Qwen2.5 Instruct as tested; Llama and Mistral use the same layout. Architectures with a different layout, such as Phi, fail to load or produce nonsense.

The package has the pieces you would expect from a small inference engine: BPE and SentencePiece tokenizers, a key/value cache, `Forward` for one step, a `Sampler` with temperature, top-k, top-p and a repetition penalty, and `Generate` for the whole loop. The cost is spelled out in the package page, and it is worth repeating plainly:

- **Memory.** Every weight is expanded to `f32` when the model loads, so Qwen2.5 0.5B, a 0.7 GB file, needs about 3 GB of memory. The key/value cache is sized from the context length, so lowering `cfg.ctx` saves memory.
- **Speed.** The documentation says to expect a few seconds per token on an ordinary machine, even in a `--release` build, which drops bounds checks from the inner loops.
- **Quality.** A half-billion-parameter model writes fluent sentences and gets facts wrong. The package's own chat example shows Qwen2.5 0.5B explaining the blue sky with water droplets.

So `ai.llm` is a tool for a narrow set of jobs: rewriting or classifying short texts offline, a local helper inside a tool where a network call is not allowed, and above all learning how a transformer works step by step, in code you can read. For anything bigger, the package page itself recommends calling llama.cpp or an API over `net.http`.

## Running ONNX models with ai.onnx {#onnx}

ONNX is the common exchange format for trained models: train in PyTorch, TensorFlow or scikit-learn, export once, and run anywhere that has an ONNX runtime. The [`ai.onnx` package](/std/ai/onnx/) is such a runtime written in Salam. `LoadOnnx` reads the file and `Run` takes one input tensor per graph input and returns the outputs as ordinary tensors.

A typical use is a small classifier exported from Python and shipped inside a native command-line tool:

```salam
import ai.onnx
import ai.tensor
import os
import conv

func main:
    args := os.Args()
    if args.len() < 6:
        println "usage: classify model.onnx f1 f2 f3 f4"
        ret 1
    end
    mut ok := false
    mut model := onnx.LoadOnnx(args.get(1), ok)
    if not ok:
        println "cannot load:", model.error
        ret 1
    end
    mut features := Vector {} as Vector<f32>
    repeat 4 in i:
        features.push(conv.ParseFloatOr(args.get(i + 2), 0.0) as f32)
    end
    mut shape := tensor.Shape2(1, 4)
    mut inputs := Vector {} as Vector<tensor.Tensor>
    inputs.push(tensor.FromArray(shape, features))
    mut outputs := onnx.Run(model, inputs, ok)
    if not ok:
        println "inference failed:", model.error
        ret 1
    end
    mut scores := outputs.get(0)
    println "output shape", tensor.ShapeString(scores)
    println "predicted class", tensor.ArgMaxAll(scores)
    model.free()
end
```

Nothing in this package stops the program on a bad file: every failure sets `ok` to false and leaves the reason in `model.error`. The limits are the ones to check before you export. Every tensor is `f32` inside, so large `int64` values lose precision. About 65 operators are implemented, enough for feed-forward, convolutional and normalization layers; `LSTM`, `Resize` and the quantized operators fail with `unsupported op`. `Conv` has no groups (so no depthwise convolutions) and no dilations. Many small classifiers and embedding models fit inside that set; many modern vision and language models do not.

## The tensor layer underneath {#tensor}

All of these packages sit on [`ai.tensor`](/std/ai/tensor/), a library of multidimensional `f32` arrays with broadcasting arithmetic, matrix products, reductions, softmax and the usual activation functions. Large operations split themselves across threads: by default one per core, at most 8.

The heart of retrieval and of attention is the same small computation: score a query against a set of vectors with a matrix product, then turn the scores into weights with softmax. Here it is with four tiny "document embeddings":

```salam
import ai.tensor
import conv

func rows(n: int, width: int, values: f64[]): tensor.Tensor:
    mut data := Vector {} as Vector<f32>
    each v in values:
        data.push(v as f32)
    end
    mut shape := tensor.Shape2(n, width)
    ret tensor.FromArray(shape, data)
end

func main:
    names := ["invoice", "receipt", "holiday", "beach"]
    mut docs := rows(4, 3, [0.9, 0.1, 0.0, 0.8, 0.2, 0.1, 0.0, 0.9, 0.4, 0.1, 0.8, 0.6])
    mut query := rows(3, 1, [1.0, 0.0, 0.1])
    mut scores := tensor.MatMul(docs, query)
    mut flat_shape := tensor.Shape1(4)
    mut flat := tensor.Reshape(scores, flat_shape)
    mut weights := tensor.Softmax(flat)
    repeat 4 in i:
        println names[i], conv.FormatFloatPrec(flat.at(i), 2), conv.FormatFloatPrec(weights.at(i), 3)
    end
    println "closest:", names[tensor.ArgMaxAll(weights)]
    tensor.Free(docs)
    tensor.Free(query)
    tensor.Free(scores)
    tensor.Free(flat)
    tensor.Free(weights)
end
```

```output
invoice 0.90 0.355
receipt 0.81 0.325
holiday 0.04 0.150
beach 0.16 0.170
closest: invoice
```

Note the `tensor.Free` calls. Salam has no garbage collector, so a tensor's buffer lives until you free it. That is extra work in a ten-line demo, and an advantage in a long-running service, where memory use is predictable and there are no collector pauses. The article on [memory management in Salam](/blog/memory-management-in-salam/) covers the rules.

On speed, the project's benchmark includes two rows that use Salam's own matrix and tensor libraries against hand-written loops in C, C++, Rust and Go. Salam's library was ahead on the tensor matrix product (7.3 ms against 38.1 ms for C++) and far behind on a row of many small matrix operations (36.4 ms against 4.1 ms for C). The [speed page](/speed/) keeps those rows out of the averages for a good reason: they compare libraries, not compilers. They say nothing about how Salam compares with an optimized BLAS or a GPU.

## Training small models on the CPU {#training}

The same stack can train. [`ai.nn`](/std/ai/nn/) has linear, convolutional, recurrent and transformer layers, `ai.autograd` records operations on a tape and computes gradients, and `ai.optim` has SGD, Adam and RMSProp. The smallest honest test is XOR, a problem a single straight line cannot solve:

```salam
import ai.nn
import ai.autograd
import ai.optim
import ai.tensor

func table(values: f64[], n: int, width: int): tensor.Tensor:
    mut data := Vector {} as Vector<f32>
    each v in values:
        data.push(v as f32)
    end
    mut shape := tensor.Shape2(n, width)
    ret tensor.FromArray(shape, data)
end

func main:
    mut x := table([0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0], 4, 2)
    mut y := table([0.0, 1.0, 1.0, 0.0], 4, 1)
    mut store := nn.ParamStore {}
    mut rng := tensor.NewRng(11)
    mut net := nn.Sequential {}
    net.add(nn.NewLinear(store, "hidden", 2, 6, rng))
    net.add(nn.TanhLayer {})
    net.add(nn.NewLinear(store, "out", 6, 1, rng))
    mut adam := optim.Adam { lr = 0.05 }
    mut tape := autograd.NewTape(1)
    repeat 600 in step:
        tape.reset()
        store.zero_grad()
        logits := net.forward(tape, store, autograd.Input(tape, x))
        loss := autograd.BCEWithLogits(tape, logits, autograd.Constant(tape, y))
        autograd.Backward(tape, loss)
        adam.step(store)
        if step % 200 == 0:
            println "step", step, "loss below 0.1:", autograd.Value(tape, loss).at(0) < 0.1
        end
    end
    tape.reset()
    probs := tensor.Sigmoid(autograd.Value(tape, net.forward(tape, store, autograd.Input(tape, x))))
    repeat 4 in i:
        println x.at2(i, 0), "xor", x.at2(i, 1), "->", (probs.at(i) + 0.5) as int
    end
    tape.free()
    adam.free()
    store.free()
end
```

```output
step 0 loss below 0.1: false
step 200 loss below 0.1: true
step 400 loss below 0.1: true
0 xor 0 -> 0
0 xor 1 -> 1
1 xor 0 -> 1
1 xor 1 -> 0
```

That is a toy, and it is meant as one. The realistic CPU training jobs are small: a tabular model, a little convolutional network, a model you fine-tune on a few thousand examples. For classic algorithms on tables (trees, k-means, logistic regression) the `ai.ml` package is shorter and needs no training loop. The [machine learning article](/blog/machine-learning-with-salam/) walks through these packages in more depth.

## Where a native language helps, and where it does not {#fit}

There are three places where running models from Salam is a real advantage rather than a curiosity.

1. **Shipping a model inside a tool.** A Salam program builds to one executable with no virtual machine. An ONNX classifier or a small embedding model can travel inside a command-line tool, a desktop app or a server without a Python install next to it.
2. **Offline and restricted environments.** Where a program cannot reach a hosted API, a small local model is the only option, and a self-contained binary is easy to move onto such a machine.
3. **Learning.** Every layer of the stack, from the GGUF parser to the attention loop, is ordinary Salam source in `std/ai/`. A student can read the whole path from bytes on disk to a sampled token, in either English or Persian, without dropping into C++ or CUDA.

And there are places where it is the wrong tool today. There is no GPU support in the library. Weights are expanded to `f32`, so memory use is several times the file size. The file formats are partly covered (no K-quants, about 65 ONNX operators). The ecosystem around these packages is tiny compared with Python's, and the packages are young. If your job is serving a large model to many users, put a dedicated inference server behind an HTTP call and use Salam for the service around it; the [web server article](/blog/fast-web-servers-with-salam/) covers that side.

## What would have to happen next {#next}

Everything in this section is vision, not a roadmap the project has published. Reading the current limits, the next useful steps are fairly obvious:

- **Memory-mapped, quantized weights.** Reading a GGUF file with `mmap` and computing on quantized blocks directly, instead of expanding to `f32`, would cut memory several times and lift the 2 GB ceiling. The standard library already has an `os.mmap` package, so the building block exists.
- **More formats.** K-quant support in `ai.gguf`, and more ONNX operators (`Resize`, grouped convolutions, the quantized operators) would let far more downloaded models load.
- **SIMD and GPU paths.** The native backend can already emit AVX2 code with `--cpu=x86-64-v3`; a tensor library that uses vector instructions deliberately, and later a GPU path, would change the speed story.
- **Benchmarks against the real competition.** The only published numbers compare Salam's tensor library with hand-written loops. A fair comparison with llama.cpp and ONNX Runtime on the same model would show exactly how far there is to go.

None of these is promised. They are the gaps a contributor could pick up, and the [ecosystem article](/blog/building-an-open-source-language-ecosystem/) explains how outside work reaches the project.

## Frequently asked questions {#faq}

### Can Salam run Llama or Qwen models?

Small ones, yes. `ai.llm` runs Llama-layout models from a GGUF file on the CPU; Qwen2.5 Instruct is the tested case. The file must be under 2 GB and use `F32`, `F16`, `Q8_0`, `Q4_0` or `Q4_1` weights, and you should expect a few seconds per token.

### Does Salam have an ONNX runtime?

Yes, `ai.onnx` reads and runs ONNX graphs in pure Salam, with no ONNX Runtime library. It implements about 65 operators, all in `f32`, which covers many feed-forward, convolutional and normalization models but not recurrent layers, `Resize` or quantized graphs.

### Does Salam support GPUs for machine learning?

No. The tensor library runs on the CPU and spreads large operations across threads. For GPU training or large-model serving, use a dedicated framework or server and call it from Salam.

### Do I need Python to use these packages?

No. The packages are part of the standard library and need no installation beyond the compiler. Python is useful only for producing the model in the first place, for example exporting a PyTorch model to ONNX or saving arrays with NumPy for the `npy` package to read.
