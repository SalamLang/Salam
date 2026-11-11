---
title: Scientific computing and machine learning in Salam: tensors, matrices, autograd and neural networks
seo_title: Machine Learning in Salam: Tensors, Autograd and Neural Networks
description: Salam ships tensors, autograd, neural network layers, optimizers, classic ML, ONNX and GGUF in its standard library. Working code, real output, honest limits.
keywords: machine learning Salam, tensors, autograd, automatic differentiation, neural network library, scientific computing language, ONNX inference, GGUF, matrix library, compiled language for machine learning
category: build
summary: Tensors, reverse-mode autograd, neural network layers, optimizers, scikit-learn style models, ONNX and GGUF readers: Salam's machine learning stack lives in the standard library and compiles to native code. A guided tour with programs that run, and a clear list of what it does not do.
date: 2026-11-11
---

Machine learning in Salam is done with packages that ship with the compiler. **The standard library includes multi-dimensional tensors, reverse-mode automatic differentiation (autograd), neural network layers, optimizers, classic machine learning models, a 64-bit matrix library, and readers for ONNX and GGUF model files**, all written in Salam and compiled to native code with no Python and no external runtime. You can train a small network, fit a decision tree, solve a linear system or run an exported ONNX model inside an ordinary Salam program.

This article tours that stack with complete programs and their real output, explains how the pieces fit, and is specific about the limits, because a machine learning library is only useful if you know where it stops.

## The machine learning packages at a glance {#packages}

| Package | What it does |
|---|---|
| [`ai.tensor`](/std/ai/tensor/) | `f32` tensors, broadcasting arithmetic, matrix products, reductions, masks |
| [`ai.autograd`](/std/ai/autograd/) | A tape that records operations and computes gradients backward |
| [`ai.nn`](/std/ai/nn/) | Layers: linear, convolution, pooling, normalization, embedding, LSTM, GRU, attention, transformer blocks, checkpoints |
| [`ai.optim`](/std/ai/optim/) | SGD with momentum, Adam and AdamW, RMSProp, gradient clipping, learning rate schedules |
| [`ai.ml`](/std/ai/ml/) | Classic models in a scikit-learn style: regression, trees, forests, boosting, SVMs, k-means, PCA, metrics |
| [`matrix`](/std/matrix/) | `f64` linear algebra: solving, decompositions, eigenvalues, SVD |
| [`ai.onnx`](/std/ai/onnx/) | Load and run ONNX models exported from other frameworks |
| [`ai.gguf`](/std/ai/gguf/), `ai.llm` | Read GGUF files and run small Llama-family language models on the CPU |
| `stats`, `data`, `npy` | Descriptive statistics, data loading and batching, NumPy `.npy` and `.npz` files |

The design is layered: `tensor` is plain arithmetic, `autograd` records operations on tensors, `nn` builds layers on the tape, and `optim` updates the weights that `nn` keeps. If you know PyTorch, the shape is familiar; if you know scikit-learn, `ai.ml` will be.

## Tensors and broadcasting {#tensors}

A tensor is a multi-dimensional array of 32-bit floats stored in row order. The `+`, `-`, `*` and `/` operators work element by element and broadcast: a dimension of size one stretches to match the other operand. This program is one dense layer by hand, a `2x3` input times a `3x2` weight matrix plus a `1x2` bias that broadcasts over both rows:

```salam
import ai.tensor

func from(rows: int, cols: int, values: f64[]): tensor.Tensor:
    mut data := Vector {} as Vector<f32>
    each v in values:
        data.push(v as f32)
    end
    mut shape := tensor.Shape2(rows, cols)
    ret tensor.FromArray(shape, data)
end

func main:
    mut x := from(2, 3, [1.0, 2.0, 3.0, 4.0, 5.0, 6.0])
    mut w := from(3, 2, [0.5, -1.0, 0.25, 0.0, 1.0, 2.0])
    bias := from(1, 2, [1.0, -9.0])
    mut y := tensor.MatMul(x, w) + bias
    println "y", tensor.ShapeString(y), tensor.ToString(y)
    println "relu", tensor.ToString(tensor.Relu(y))
    println "row sums", tensor.ToString(tensor.SumAxis(y, 1, false))
end
```

```output
y 2x2 tensor(2x2)[5 -4 10.25 -1]
relu tensor(2x2)[5 0 10.25 0]
row sums tensor(2)[1 9.25]
```

Large operations spread over threads on their own: element-wise work from 262,144 values and matrix products above about 32 million multiply-adds, using one thread per core up to 8 by default. `SetThreads(1)` keeps everything on the calling thread.

> [!WARNING]
> A shape mismatch does not stop the program. The result is an empty tensor, and `ShapeString` prints `empty`. Check `is_empty()` while you are unsure of your shapes, the same way you would watch for silent broadcasting surprises in NumPy.

## Automatic differentiation with a tape {#autograd}

Training means lowering a loss by following its gradient. `ai.autograd` records every operation on a `Tape`, and `Backward` walks the tape in reverse to compute the gradient of the loss for every input. Functions return node indices; `Value` and `Grad` read the tensors behind them.

The smallest useful example is gradient descent on one number. The loss is `(w - 3)` squared, so the gradient is `2(w - 3)` and the minimum is at 3:

```salam
import ai.autograd
import ai.tensor
import conv

func main:
    mut tape := autograd.NewTape(1)
    mut start := Vector {} as Vector<f32>
    start.push(0.0 as f32)
    mut shape := tensor.Shape1(1)
    mut w := tensor.FromArray(shape, start)
    repeat 31 in step:
        tape.reset()
        x := autograd.Watch(tape, w)
        diff := autograd.AddScalar(tape, x, -3.0)
        loss := autograd.Sum(tape, autograd.Mul(tape, diff, diff))
        autograd.Backward(tape, loss)
        if step % 10 == 0:
            value := autograd.Value(tape, loss).at(0) as f64
            println "step", step, "w", conv.FormatFloatPrec(w.at(0) as f64, 4), "loss", conv.FormatFloatPrec(value, 6)
        end
        mut g := autograd.Grad(tape, x)
        tensor.AddScaled(w, g, -0.1)
    end
    tape.free()
end
```

```output
step 0 w 0.0000 loss 9.000000
step 10 w 2.6779 loss 0.103763
step 20 w 2.9654 loss 0.001196
step 30 w 2.9963 loss 0.000014
```

With a learning rate of 0.1 the distance to the minimum shrinks by a factor of 0.8 per step, so after ten steps `w` is `3(1 - 0.8^10)`, about 2.678, which is exactly what the program prints. Two rules from the package documentation matter in real code: call `reset` at the start of every step, which frees the previous step's tensors, and zero your gradients, because they are added, never replaced. `GradCheck` compares a gradient against a numerical estimate when you write a new layer or loss.

## Training a neural network {#neural-network}

`ai.nn` provides the layers, all keeping their weights in one `ParamStore`, and `ai.optim` provides the update rule. XOR is the classic first test, because no single straight line separates its classes and a hidden layer is required:

```salam
import ai.nn
import ai.autograd
import ai.optim
import ai.tensor

func table(rows: int, cols: int, values: f64[]): tensor.Tensor:
    mut data := Vector {} as Vector<f32>
    each v in values:
        data.push(v as f32)
    end
    mut shape := tensor.Shape2(rows, cols)
    ret tensor.FromArray(shape, data)
end

func main:
    mut x := table(4, 2, [0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0])
    mut y := table(4, 1, [0.0, 1.0, 1.0, 0.0])
    mut store := nn.ParamStore {}
    mut rng := tensor.NewRng(42)
    mut model := nn.Sequential {}
    model.add(nn.NewLinear(store, "hidden", 2, 16, rng))
    model.add(nn.ReluLayer {})
    model.add(nn.NewLinear(store, "out", 16, 1, rng))
    mut opt := optim.Adam { lr = 0.03 }
    mut tape := autograd.NewTape(1)
    repeat 600 in step:
        tape.reset()
        store.zero_grad()
        logits := model.forward(tape, store, autograd.Input(tape, x))
        loss := autograd.BCEWithLogits(tape, logits, autograd.Constant(tape, y))
        autograd.Backward(tape, loss)
        opt.step(store)
        if step % 200 == 0:
            println "step", step, "loss below 0.1:", autograd.Value(tape, loss).at(0) < 0.1
        end
    end
    tape.reset()
    probs := tensor.Sigmoid(autograd.Value(tape, model.forward(tape, store, autograd.Input(tape, x))))
    repeat 4 in i:
        println x.at2(i, 0), "xor", x.at2(i, 1), "->", (probs.at(i) + 0.5) as int
    end
    tape.free()
    opt.free()
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

The loop is the same five lines in every training program: reset the tape, zero the gradients, run the forward pass and the loss, call `Backward`, and step the optimizer. The seed passed to `NewRng` fixes the starting weights, so a run repeats exactly. Checkpoints save a `ParamStore` by layer name (gzip-compressed when the path ends in `.gz`), and loading checks the CRC, the parameter count, every name and every shape before writing any weight. Notice also the explicit `free` calls at the end: Salam has no garbage collector, so long training jobs should release tensors they no longer need, as described in [memory management in Salam](/blog/memory-management-in-salam/).

## Classic machine learning with ai.ml {#classic-ml}

Many practical problems need a decision tree or a logistic regression, not a neural network. `ai.ml` follows the scikit-learn pattern: data is a `matrix` with one sample per row, every model is a struct with `fit` and `predict`, and the metrics, scalers, splitters, cross-validation and grid search are there too. A shallow tree that predicts whether a student passes from hours of study and hours of sleep:

```salam
import ai.ml
import matrix

func build(rows: int, cols: int, numbers: str): matrix.Matrix:
    mut data := Vector {} as Vector<f64>
    each number in numbers.split(" "):
        data.push(number.to_float())
    end
    ret matrix.FromArray(rows, cols, data)
end

func main:
    mut hours := build(10, 2, "1 4 2 5 2 8 3 6 4 7 5 7 6 8 7 6 8 7 9 8")
    mut passed := build(10, 1, "0 0 0 0 1 1 1 1 1 1")
    mut tree := ml.DecisionTree { max_depth = 2 }
    defer tree.free()
    println "trained:", tree.fit(hours, passed), "nodes:", tree.node_count()
    mut pred := tree.predict(hours)
    println "training accuracy:", ml.Accuracy(passed, pred)
    mut students := build(3, 2, "1 9 5 5 10 4")
    mut guess := tree.predict(students)
    repeat 3 in i:
        outcome := guess.at(i, 0) > 0.5 ? "pass" : "fail"
        println "study", students.at(i, 0), "sleep", students.at(i, 1), "->", outcome
    end
end
```

```output
trained: true nodes: 3
training accuracy: 1
study 1 sleep 9 -> fail
study 5 sleep 5 -> pass
study 10 sleep 4 -> pass
```

The tree found the one split that matters (study hours), and three nodes are enough. Training accuracy of 1 on ten samples proves nothing about new data, which is why the package also has `TrainTestSplit`, `StratifiedKFold` and `CrossValScore`. The model list is long: linear, ridge, lasso and logistic regression, decision trees, random forests, gradient boosting and histogram gradient boosting, AdaBoost, linear and kernel SVMs, k-nearest neighbors, naive Bayes, LDA and QDA, k-means, DBSCAN, Gaussian mixtures, agglomerative clustering, PCA, truncated SVD and t-SNE.

## Exact linear algebra with matrix {#matrix}

Tensors are `f32` because neural networks need speed more than the last digits. Engineering, statistics and finance usually need the opposite, so the `matrix` package works in 64-bit floats: `Solve` for one system, `Decompose` once and `LUSolve` many times for several, plus eigenvalues, singular values and principal component analysis. Like tensors, it reports failure with an empty matrix instead of stopping, so check `is_empty()` after `Solve` on a matrix that might be singular.

## Running models trained elsewhere: ONNX and GGUF {#onnx-gguf}

Most teams train in Python. `ai.onnx` lets a Salam program run the exported model with no ONNX Runtime library: `LoadOnnx` reads the file and `Run` returns ordinary tensors. The limits are documented plainly. About 65 operators are implemented, which covers feed-forward, convolutional and normalization layers; `LSTM`, `Resize` and the quantized operators fail with `unsupported op`. Every value is `f32` inside, and weights stored in a separate external data file do not load.

`ai.gguf` reads llama.cpp's GGUF format, and `ai.llm` runs Llama-family models such as Qwen2.5 on the processor. The file must be under 2 GB and use `F32`, `F16`, `Q8_0`, `Q4_0` or `Q4_1` weights; the popular K-quants are not supported. Weights are expanded to `f32` on load, so a 0.7 GB Qwen2.5 0.5B file needs about 3 GB of memory. The package documentation itself recommends calling a dedicated inference server for large models or high speed. The article on [AI workloads with Salam](/blog/ai-workloads-with-salam/) goes into that side in more depth.

## Performance: what the benchmark says {#performance}

The project's [benchmark](/speed/) includes three programs built on these libraries. They are kept out of the averages, because Salam calls its standard library while the C, C++, Rust and Go versions use hand-written loops, so they compare libraries rather than compilers. Salam was ahead on two: a `matrix` multiplication took 14.3 ms against 42.5 ms for C, and a `tensor` multiplication 7.3 ms against 38.1 ms for C++. It was far behind on the third, a series of small matrix operations, at 36.4 ms against 4.1 ms for C, where gcc removes most of the loop work. Read that as "a tuned library beats a naive loop and loses when the optimizer can delete the work", not as a general speed ranking. The [Salam vs Python and Julia](/blog/salam-vs-python-and-julia/) article compares the numerical side with those ecosystems.

## What Salam is not, for machine learning {#limits}

- **No GPU.** Everything runs on the CPU. There is no CUDA, Metal or Vulkan back end, so training anything beyond small models is impractical.
- **A small ecosystem.** PyTorch, JAX, scikit-learn and Hugging Face have years of models, tutorials and pretrained weights. Salam has a standard library and no third-party model zoo. Python remains the right place for research.
- **`f32` tensors only.** No `f16` or `bf16` training, and integer weights are converted to `f32`.
- **Manual memory.** Every operation makes a new tensor. In a long loop, use the `...Into` and `...InPlace` forms or free results explicitly.
- **Young code.** These packages are recent, have few users outside the project, and their APIs can still change before 1.0.

Where Salam fits is the other end of the pipeline: a single native executable that loads a trained model and serves it over HTTP, scores rows from a database, or embeds a small classifier in a command-line tool, with no Python environment to install on the target machine. It is also a readable place to learn how autograd and transformers work, because every layer is ordinary Salam source in `std/ai/`. The [standard library tour](/blog/salam-standard-library-tour/) shows what else ships next to these packages.

## Frequently asked questions {#faq}

### Can I train a neural network in Salam?

Yes. `ai.nn` has linear, convolutional, recurrent, attention and transformer layers, `ai.autograd` computes gradients and `ai.optim` has SGD, Adam, AdamW and RMSProp. Training runs on the CPU, so it suits small and medium models rather than large ones.

### Does Salam support GPUs for machine learning?

No. All tensor operations run on the CPU, with large operations split across threads. There is no CUDA or other GPU back end.

### Can Salam load a PyTorch model?

Not directly. Export it to ONNX and load it with `ai.onnx`, as long as the model only uses the roughly 65 supported operators and keeps its weights inside the `.onnx` file. For language models, `ai.llm` loads GGUF files of the Llama family.

### Is Salam a replacement for Python in data science?

Not today. Python's libraries, notebooks and community are far larger. Salam is useful when you want a compiled program without a Python runtime, or when you want to read and learn from a complete machine learning stack written in one language.
