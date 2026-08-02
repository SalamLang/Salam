; ModuleID = 'll_constfold'
source_filename = "ll_constfold.salam"

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

; Salam -> LLVM IR (textual). usize = i64 (64-bit target).
declare i32 @printf(ptr, ...) nounwind
declare i32 @dprintf(i32, ptr, ...) nounwind
declare i64 @write(i32, ptr, i64) nounwind
declare i64 @strlen(ptr) nounwind willreturn memory(argmem: read)
declare i32 @strcmp(ptr, ptr) nounwind willreturn memory(argmem: read)
declare noalias ptr @malloc(i64) nounwind allocsize(0)
declare noalias ptr @realloc(ptr, i64) nounwind allocsize(1)
declare void @free(ptr) nounwind
declare ptr @memcpy(ptr, ptr, i64) nounwind
declare ptr @memmove(ptr, ptr, i64) nounwind
declare void @abort() noreturn nounwind cold
declare void @exit(i32) noreturn nounwind
declare i32 @snprintf(ptr, i64, ptr, ...) nounwind
declare i64 @strtol(ptr, ptr, i32) nounwind
declare double @strtod(ptr, ptr) nounwind
declare ptr @strstr(ptr, ptr) nounwind willreturn memory(argmem: read)
declare double @llvm.pow.f64(double, double)
%dyn = type { ptr, ptr }

@.str.0 = private unnamed_addr constant [14 x i8] c"salam panic: \00"
@.str.1 = private unnamed_addr constant [4 x i8] c"%s\0A\00"
define void @salam_panic(ptr %arg0) noreturn nounwind {
entry:
  %p.msg = alloca ptr
  store ptr %arg0, ptr %p.msg
  %t0 = load ptr, ptr %p.msg
  %t1 = call ptr @salam_ll_strcat(ptr @.str.0, ptr %t0)
  %t2 = call i32 (ptr, ...) @printf(ptr @.str.1, ptr %t1)
  call void @abort()
  ret void
}

@.str.2 = private unnamed_addr constant [20 x i8] c"salam: array index \00"

define internal ptr @salam_FormatUint_u64(i64 %arg0) nounwind {
entry:
  %p.x = alloca i64
  %v.nd.0 = alloca i32
  %v.t.1 = alloca i64
  %v.buf.9 = alloca ptr
  %v.p.14 = alloca ptr
  %v.v.21 = alloca i64
  %v.i.23 = alloca i32
  store i64 %arg0, ptr %p.x
  store i32 1, ptr %v.nd.0
  %t2 = load i64, ptr %p.x
  store i64 %t2, ptr %v.t.1
  br label %L0_wcond
L0_wcond:
  %t3 = load i64, ptr %v.t.1
  %t4 = icmp uge i64 %t3, 10
  br i1 %t4, label %L1_wbody, label %L2_wend
L1_wbody:
  %t5 = load i64, ptr %v.t.1
  %t6 = udiv i64 %t5, 10
  store i64 %t6, ptr %v.t.1
  %t7 = load i32, ptr %v.nd.0
  %t8 = add i32 %t7, 1
  store i32 %t8, ptr %v.nd.0
  br label %L0_wcond
L2_wend:
  %t10 = load i32, ptr %v.nd.0
  %t11 = add i32 %t10, 1
  %t12 = sext i32 %t11 to i64
  %t13 = call ptr @malloc(i64 %t12)
  store ptr %t13, ptr %v.buf.9
  %t15 = load ptr, ptr %v.buf.9
  store ptr %t15, ptr %v.p.14
  %t17 = load ptr, ptr %v.p.14
  %t18 = load i32, ptr %v.nd.0
  %t19 = sext i32 %t18 to i64
  %t20 = getelementptr inbounds i8, ptr %t17, i64 %t19
  store i8 0, ptr %t20
  %t22 = load i64, ptr %p.x
  store i64 %t22, ptr %v.v.21
  %t24 = load i32, ptr %v.nd.0
  %t25 = sub i32 %t24, 1
  store i32 %t25, ptr %v.i.23
  br label %L3_wcond
L3_wcond:
  %t26 = load i32, ptr %v.i.23
  %t27 = icmp sge i32 %t26, 0
  br i1 %t27, label %L4_wbody, label %L5_wend
L4_wbody:
  %t28 = load ptr, ptr %v.p.14
  %t29 = load i32, ptr %v.i.23
  %t30 = sext i32 %t29 to i64
  %t31 = getelementptr inbounds i8, ptr %t28, i64 %t30
  %t32 = load i64, ptr %v.v.21
  %t33 = urem i64 %t32, 10
  %t34 = trunc i64 %t33 to i32
  %t35 = add i32 %t34, 48
  %t36 = trunc i32 %t35 to i8
  store i8 %t36, ptr %t31
  %t37 = load i64, ptr %v.v.21
  %t38 = udiv i64 %t37, 10
  store i64 %t38, ptr %v.v.21
  %t39 = load i32, ptr %v.i.23
  %t40 = sub i32 %t39, 1
  store i32 %t40, ptr %v.i.23
  br label %L3_wcond
L5_wend:
  %t41 = load ptr, ptr %v.buf.9
  ret ptr %t41
}

@.str.3 = private unnamed_addr constant [2 x i8] c"-\00"
define internal ptr @salam_FormatInt_i64(i64 %arg0) nounwind {
entry:
  %p.n = alloca i64
  %v.mag.5 = alloca i64
  store i64 %arg0, ptr %p.n
  %t0 = load i64, ptr %p.n
  %t1 = icmp sge i64 %t0, 0
  br i1 %t1, label %L0_then, label %L1_endif
L0_then:
  %t2 = load i64, ptr %p.n
  %t4 = call ptr @salam_FormatUint_u64(i64 %t2)
  ret ptr %t4
L1_endif:
  %t6 = load i64, ptr %p.n
  %t8 = sub i64 0, %t6
  store i64 %t8, ptr %v.mag.5
  %t9 = load i64, ptr %v.mag.5
  %t10 = call ptr @salam_FormatUint_u64(i64 %t9)
  %t11 = call ptr @salam_ll_strcat(ptr @.str.3, ptr %t10)
  ret ptr %t11
}

@.str.4 = private unnamed_addr constant [19 x i8] c" out of bounds [0,\00"
@.str.5 = private unnamed_addr constant [2 x i8] c")\00"
define i64 @salam_idx(i64 %arg0, i64 %arg1) nounwind {
entry:
  %p.idx = alloca i64
  %p.len = alloca i64
  store i64 %arg0, ptr %p.idx
  store i64 %arg1, ptr %p.len
  %t0 = load i64, ptr %p.idx
  %t1 = icmp slt i64 %t0, 0
  br i1 %t1, label %L1_sc_short, label %L0_sc_rhs
L1_sc_short:
  br label %L3_sc_end
L0_sc_rhs:
  %t2 = load i64, ptr %p.idx
  %t3 = load i64, ptr %p.len
  %t4 = icmp sge i64 %t2, %t3
  br label %L2_sc_join
L2_sc_join:
  br label %L3_sc_end
L3_sc_end:
  %t5 = phi i1 [ true, %L1_sc_short ], [ %t4, %L2_sc_join ]
  br i1 %t5, label %L4_then, label %L5_endif
L4_then:
  %t6 = load i64, ptr %p.idx
  %t7 = call ptr @salam_FormatInt_i64(i64 %t6)
  %t8 = call ptr @salam_ll_strcat(ptr @.str.2, ptr %t7)
  %t9 = call ptr @salam_ll_strcat(ptr %t8, ptr @.str.4)
  %t10 = load i64, ptr %p.len
  %t11 = call ptr @salam_FormatInt_i64(i64 %t10)
  %t12 = call ptr @salam_ll_strcat(ptr %t9, ptr %t11)
  %t13 = call ptr @salam_ll_strcat(ptr %t12, ptr @.str.5)
  %t14 = call i32 (ptr, ...) @printf(ptr @.str.1, ptr %t13)
  call void @abort()
  br label %L5_endif
L5_endif:
  %t15 = load i64, ptr %p.idx
  ret i64 %t15
}

@.str.6 = private unnamed_addr constant [7 x i8] c"concat\00"
@.str.7 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@.str.8 = private unnamed_addr constant [4 x i8] c"%g\0A\00"
@.str.9 = private unnamed_addr constant [5 x i8] c"true\00"
@.str.10 = private unnamed_addr constant [6 x i8] c"false\00"
define i32 @main() nounwind {
entry:
  %v.a.0 = alloca i32
  %v.b.1 = alloca double
  %v.s.2 = alloca ptr
  %v.t.3 = alloca i1
  %v.u.4 = alloca i32
  %v.down.5 = alloca i32
  %t6 = alloca i64
  %v.i.7 = alloca i32
  store i32 17, ptr %v.a.0
  store double 3.25, ptr %v.b.1
  store ptr @.str.6, ptr %v.s.2
  store i1 true, ptr %v.t.3
  store i32 -15, ptr %v.u.4
  store i32 0, ptr %v.down.5
  store i64 10, ptr %t6
  br label %L0_rcond
L0_rcond:
  %t8 = load i64, ptr %t6
  %t9 = icmp sge i64 %t8, 1
  br i1 %t9, label %L1_rbody, label %L3_rend
L1_rbody:
  %t10 = trunc i64 %t8 to i32
  store i32 %t10, ptr %v.i.7
  %t11 = load i32, ptr %v.down.5
  %t12 = load i32, ptr %v.i.7
  %t13 = add i32 %t11, %t12
  store i32 %t13, ptr %v.down.5
  br label %L2_rstep
L2_rstep:
  %t14 = load i64, ptr %t6
  %t15 = add i64 %t14, -1
  store i64 %t15, ptr %t6
  br label %L0_rcond
L3_rend:
  %t16 = load i32, ptr %v.a.0
  %t17 = call i32 (ptr, ...) @printf(ptr @.str.7, i32 %t16)
  %t18 = load double, ptr %v.b.1
  %t19 = call i32 (ptr, ...) @printf(ptr @.str.8, double %t18)
  %t20 = load ptr, ptr %v.s.2
  %t21 = call i32 (ptr, ...) @printf(ptr @.str.1, ptr %t20)
  %t22 = load i1, ptr %v.t.3
  %t23 = select i1 %t22, ptr @.str.9, ptr @.str.10
  %t24 = call i32 (ptr, ...) @printf(ptr @.str.1, ptr %t23)
  %t25 = load i32, ptr %v.u.4
  %t26 = call i32 (ptr, ...) @printf(ptr @.str.7, i32 %t25)
  %t27 = load i32, ptr %v.down.5
  %t28 = call i32 (ptr, ...) @printf(ptr @.str.7, i32 %t27)
  ret i32 0
}

; on-demand runtime helpers
define internal noalias ptr @salam_ll_strcat(ptr %a, ptr %b) nounwind willreturn nofree {
entry:
  %la = call i64 @strlen(ptr %a)
  %lb = call i64 @strlen(ptr %b)
  %sum = add i64 %la, %lb
  %tot = add i64 %sum, 1
  %buf = call ptr @malloc(i64 %tot)
  %c1 = call ptr @memcpy(ptr %buf, ptr %a, i64 %la)
  %end = getelementptr i8, ptr %buf, i64 %la
  %lb1 = add i64 %lb, 1
  %c2 = call ptr @memcpy(ptr %end, ptr %b, i64 %lb1)
  ret ptr %buf
}

