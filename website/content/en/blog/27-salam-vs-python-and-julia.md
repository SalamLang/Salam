---
title: Salam vs Python and Julia for scientific computing and numerical work
seo_title: Salam vs Python and Julia for Scientific Computing
description: Salam vs Python and Julia for numerical computing: linear algebra, statistics and regression side by side, plus an honest reading of the matrix benchmark rows.
keywords: scientific computing language, Julia alternative, numerical computing, Salam vs Python, Salam vs Julia, NumPy alternative, linear algebra library, statistics programming language, compiled language for data science
category: compare
summary: Python owns scientific computing through NumPy and its ecosystem, and Julia was designed for it. Salam brings a native compiler and a pure-Salam numerics library. This is what that library can do, what the benchmark rows measure, and where each language belongs.
date: 2026-11-11
---

Which language should you use for scientific computing and numerical work: Python, Julia or something compiled? **Python is the dominant language of science and data work, fast in practice because NumPy, SciPy and PyTorch run their heavy loops in compiled C, C++ and Fortran. Julia is a language designed for numerical computing, compiled just in time through LLVM, with multiple dispatch and loops that run at compiled speed. Salam is a general-purpose compiled language whose standard library includes dense linear algebra, statistics, tensors, automatic differentiation and machine learning, all written in Salam.** Each one makes sense for a different kind of project.

This article puts the same numerical tasks side by side in the three languages, reads the three matrix and tensor rows of Salam's benchmark carefully, and is honest about how far Salam is from the Python and Julia ecosystems. The Salam programs are compiled and run when this site is built.

## Three approaches to fast numerics {#approaches}

Python itself is slow at arithmetic loops. On Salam's benchmark of 39 programs that use the same algorithm in every language, pure Python took about 56 times as long as Salam. Nobody does serious numerics in pure Python, though: NumPy arrays hand whole operations to compiled code and to optimized BLAS and LAPACK libraries, so a matrix product in NumPy runs at native speed. The cost appears when the work cannot be expressed as a few large array operations, in loops over many small pieces, where each NumPy call pays interpreter overhead.

Julia removes that split. Ordinary Julia loops are compiled to machine code by an LLVM-based JIT, specialized on the types they are called with, so you can write the loop yourself and still get compiled speed. The cost is compilation latency: the first call of a function takes time to compile, which Julia has reduced substantially since version 1.9 by caching native code for packages.

Salam compiles ahead of time to a native executable, like C or Rust. Loops are fast from the first run, there is no interpreter and no JIT warm-up, and the numerical packages are ordinary Salam code. The cost is the ecosystem: Salam's numerics are one standard library, not the tens of thousands of packages Python and Julia have.

|                             | Python (NumPy)                                      | Julia                                   | Salam                                        |
| --------------------------- | --------------------------------------------------- | --------------------------------------- | -------------------------------------------- |
| Execution                   | Interpreter; array operations in compiled libraries | JIT compiled through LLVM               | Ahead-of-time native code                    |
| Fast loops in your own code | Only through NumPy, Numba, Cython or similar        | Yes                                     | Yes                                          |
| Linear algebra              | NumPy and SciPy on BLAS/LAPACK                      | Built-in `LinearAlgebra` on BLAS/LAPACK | `matrix` package, pure Salam                 |
| Machine learning            | PyTorch, JAX, scikit-learn                          | Flux, Lux, MLJ                          | `ai.tensor`, `ai.autograd`, `ai.nn`, `ai.ml` |
| GPU                         | Yes, widely                                         | Yes                                     | No                                           |
| Notebooks and plotting      | Jupyter, Matplotlib and many more                   | Jupyter, Pluto, Plots, Makie            | None built-in                                |
| Ecosystem                   | Enormous                                            | Large in science                        | One standard library                         |

## Solving a linear system {#linear-system}

The first task: solve the 2-by-2 system `4x + y = 1`, `2x + 3y = 2`. In NumPy and Julia it is one line each:

```python
import numpy as np

A = np.array([[4.0, 1.0], [2.0, 3.0]])
b = np.array([1.0, 2.0])
x = np.linalg.solve(A, b)
print(x, np.linalg.det(A))
```

```julia
using LinearAlgebra

A = [4.0 1.0; 2.0 3.0]
b = [1.0, 2.0]
x = A \ b
println(x, " ", det(A))
```

Salam has no matrix literal syntax, so the matrix is built from a vector of numbers with [`matrix.FromArray`](/std/matrix/). `Solve` uses an LU decomposition with pivoting, like the other two, and returns an empty matrix instead of stopping the program when the system has no unique solution:

```salam
import matrix

func main:
    mut cells := Vector {} as Vector<f64>
    each v in [4.0, 1.0, 2.0, 3.0]:
        cells.push(v)
    end
    mut rhs := Vector {} as Vector<f64>
    each v in [1.0, 2.0]:
        rhs.push(v)
    end
    mut a := matrix.FromArray(2, 2, cells)
    mut b := matrix.FromArray(2, 1, rhs)
    mut x := matrix.Solve(a, b)
    if x.is_empty():
        println "no unique solution"
        ret 1
    end
    println "x =", matrix.ToStringPrec(matrix.Transpose(x), 4)
    println "det =", matrix.Det(a)
    a.Free()
    b.Free()
    x.Free()
end
```

```output
x = 0.1000 0.6000
det = 10
```

The Salam version is clearly longer. Building matrices, checking for an empty result and freeing each one with `.Free()` are things Python and Julia do for you with literals, exceptions and a garbage collector. That is the general pattern of this comparison: Salam's numerics are capable, but the language was not designed around arrays the way Julia was, or adapted to them over two decades the way Python was.

The `matrix` package goes well beyond this: LU, QR, Cholesky and SVD decompositions, symmetric and general eigenvalues, rank, pseudo-inverse, null space, matrix exponential and square root, iterative solvers (conjugate gradient, Jacobi, power iteration), norms, covariance, correlation and PCA. Everything works on 64-bit floats in one row-major `Matrix` type.

## Descriptive statistics {#statistics}

For summary statistics, Python has the `statistics` module and NumPy, and Julia the `Statistics` standard library:

```python
import numpy as np

temps = np.array([21.5, 23.0, 19.8, 25.1, 22.4, 30.2, 20.9])
print(temps.mean(), np.median(temps), temps.std(ddof=1), np.percentile(temps, 90))
```

```julia
using Statistics

temps = [21.5, 23.0, 19.8, 25.1, 22.4, 30.2, 20.9]
println(mean(temps), " ", median(temps), " ", std(temps), " ", quantile(temps, 0.9))
```

Salam's [stats package](/std/stats/) works on a `Vector<f64>`. Like Julia's `std` and NumPy with `ddof=1`, `Stdev` is the sample standard deviation, and `Percentile` interpolates linearly between the two nearest values, which is also what NumPy and Julia do by default:

```salam
import conv
import stats

func show(label: str, x: f64):
    println label, conv.FormatFloatPrec(x, 2)
end

func main:
    mut temps := Vector {} as Vector<f64>
    defer temps.Free()
    each t in [21.5, 23.0, 19.8, 25.1, 22.4, 30.2, 20.9]:
        temps.push(t)
    end
    show("mean:  ", stats.Mean(temps))
    show("median:", stats.Median(temps))
    show("stdev: ", stats.Stdev(temps))
    show("p90:   ", stats.Percentile(temps, 90.0))
end
```

```output
mean:   23.27
median: 22.40
stdev:  3.49
p90:    27.14
```

The package is small: sum, mean, median, minimum, maximum, sample and population variance and standard deviation, geometric mean and percentiles. One detail matters in practice: an empty vector gives 0 rather than an error, so check the length first. There is no equivalent of SciPy's statistical tests or distributions, and no data frame like pandas or DataFrames.jl.

## Fitting a line {#regression}

Least-squares regression shows the same pattern. NumPy fits a polynomial in one call, and Julia solves the overdetermined system with the same backslash operator it uses for square ones:

```python
import numpy as np

hours = np.array([1.0, 2.0, 3.0, 4.0, 5.0])
score = np.array([52.0, 57.0, 66.0, 70.0, 77.0])
slope, intercept = np.polyfit(hours, score, 1)
print(intercept, slope)
```

```julia
hours = [1.0, 2.0, 3.0, 4.0, 5.0]
score = [52.0, 57.0, 66.0, 70.0, 77.0]
X = [ones(length(hours)) hours]
intercept, slope = X \ score
println(intercept, " ", slope)
```

Salam's `matrix.LinearRegression` adds the intercept column and solves the least-squares problem:

```salam
import matrix

func column(values: f64[]): matrix.Matrix:
    mut data := Vector {} as Vector<f64>
    each v in values:
        data.push(v)
    end
    ret matrix.FromArray(values.len(), 1, data)
end

func main:
    mut hours := column([1.0, 2.0, 3.0, 4.0, 5.0])
    mut score := column([52.0, 57.0, 66.0, 70.0, 77.0])
    mut fit := matrix.LinearRegression(hours, score)
    println "score =", matrix.ToStringPrec(matrix.Full(1, 1, fit.at(0, 0)), 2), "+", matrix.ToStringPrec(matrix.Full(1, 1, fit.at(1, 0)), 2), "* hours"
    hours.Free()
    score.Free()
    fit.Free()
end
```

```output
score = 45.50 + 6.30 * hours
```

For more than a line, the [ai.ml package](/std/ai/ml/) follows scikit-learn's shape: a struct per estimator with `fit`, `predict` and `score`, covering linear and regularized regression, logistic regression, k-nearest neighbors, naive Bayes, decision trees, random forests, gradient boosting, support vector machines, k-means, DBSCAN, Gaussian mixtures, PCA, t-SNE and cross-validation.

## Tensors, autograd and neural networks {#tensors}

Beyond 64-bit linear algebra, Salam has an `f32` tensor package ([ai.tensor](/std/ai/tensor/)) with broadcasting, reductions, convolution and matrix products that use several threads for large inputs, a tape-based reverse-mode automatic differentiation package ([ai.autograd](/std/ai/autograd/)), neural network layers up to LSTM, GRU, multi-head attention and transformer blocks (ai.nn), optimizers such as SGD, Adam and AdamW, loaders for ONNX models and GGUF weights, and readers and writers for NumPy's `.npy` and `.npz` files, which is the practical bridge if you prepare data in Python. All of it is pure Salam, seeded and deterministic: the same seed gives the same result on every machine.

It runs on the CPU only. There is no GPU support, no mixed precision and nothing like the scale of PyTorch or JAX, so training large models is out of scope. The article [Scientific computing and machine learning in Salam](/blog/machine-learning-with-salam/) shows these packages in use.

## What the matrix benchmark rows really show {#benchmark}

Salam's [speed page](/speed/) lists 42 programs. Three of them, rows 40 to 42, call Salam's own matrix and tensor library, while the C, C++, Rust and Go versions are written as straightforward loops, and the Python versions use NumPy. The page says plainly that these rows compare libraries, not compilers, and leaves them out of its averages. Here they are, in milliseconds, fastest of 10 runs:

| Program                                             | C    | C++  | Rust | Go   | Salam | Python (NumPy) |
| --------------------------------------------------- | ---- | ---- | ---- | ---- | ----- | -------------- |
| 40, 400x400 `f64` matrix product (`matrix.MatMul`)  | 42.5 | 43.8 | 64.3 | 47.2 | 14.3  | 121.8          |
| 41, 200,000 small 4x4 operations (`matrix` package) | 4.1  | 5.0  | 5.2  | 15.9 | 36.4  | 1,174.4        |
| 42, 400x400 `f32` tensor product (`tensor.MatMul`)  | 40.3 | 38.1 | 55.7 | 43.0 | 7.3   | 119.1          |

How to read them:

- **Rows 40 and 42** favor Salam because a library product is being compared with a simple triple loop. Salam's `matrix.MatMul` works in cache-sized blocks, and a tensor product of this size runs on several threads. A C program that called an optimized BLAS would look very different. These rows say that Salam's library is reasonably well engineered, not that Salam compiles loops better than C.
- **Row 41** goes the other way. Two hundred thousand tiny products mean hundreds of thousands of heap allocations and general-purpose library calls, while the C version keeps two 4x4 arrays on the stack and gcc removes most of the loop work. Salam took 36.4 ms against 4.1 ms for C. For many small fixed-size matrices, plain arrays in Salam are the better choice.
- **Python with NumPy** is far closer to the compiled languages in rows 40 and 42 (about 120 ms) than its usual 56-times gap, because NumPy does the multiplication in compiled code. Row 41 shows the opposite case: per-call overhead on many small arrays, 1,174 ms.

Julia is not part of this benchmark, so there is no measured comparison with it. Every program in the suite is short, process start-up is part of each time, and the runs were on a shared CI virtual machine, so small gaps mean nothing. The [benchmark article](/blog/is-salam-faster-than-c-and-rust/) covers the method.

## When to choose which {#choose}

**Choose Python when:**

- you need the ecosystem: SciPy, pandas, scikit-learn, PyTorch, JAX, Jupyter and plotting;
- you explore data interactively and iterate quickly;
- your work is large array operations or GPU training, where the libraries do the heavy lifting.

**Choose Julia when:**

- you write new numerical algorithms whose inner loops do not fit array operations;
- you want mathematical syntax, multiple dispatch and a strong scientific package ecosystem (differential equations, optimization, probabilistic programming);
- long-running computations make JIT latency irrelevant.

**Choose Salam when:**

- numerics are one part of a larger native program, such as a server, a command-line tool or an embedded system, and you want them in the same language and binary;
- you need a self-contained executable with no interpreter, runtime or package environment to install;
- deterministic CPU results, small models and classical machine learning are enough;
- you teach numerical methods in Persian or English and want students to read every line of the library.

## Frequently asked questions {#faq}

### Can Salam replace NumPy?

For moderate dense linear algebra and statistics inside a compiled program, the `matrix`, `stats` and `ai.tensor` packages cover much of the same ground. They have no GPU support, no BLAS backend and no ecosystem comparable to SciPy and pandas, so for data science work NumPy remains the better tool.

### Is Salam faster than Julia?

There is no published measurement comparing them. Both produce native code through LLVM for hot loops; Julia does it at runtime and Salam ahead of time, so Salam avoids JIT latency while Julia's specialization and its BLAS-backed linear algebra are mature and fast.

### Does Salam have a REPL or notebooks for exploration?

Salam has a REPL (`salam cli`) and an online editor, but no notebook environment like Jupyter or Pluto and no plotting library.

### Can Salam read data prepared in Python?

Yes. The `npy` package reads and writes NumPy `.npy` and `.npz` files, and the `data` package loads CSV tables into matrices, so data can move between a Python workflow and a Salam program.
