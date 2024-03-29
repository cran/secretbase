
<!-- README.md is generated from README.Rmd. Please edit that file -->

# secretbase

<!-- badges: start -->

[![CRAN
status](https://www.r-pkg.org/badges/version/secretbase?color=42147b)](https://CRAN.R-project.org/package=secretbase)
[![secretbase status
badge](https://shikokuchuo.r-universe.dev/badges/secretbase?color=e4723a)](https://shikokuchuo.r-universe.dev/secretbase)
[![R-CMD-check](https://github.com/shikokuchuo/secretbase/workflows/R-CMD-check/badge.svg)](https://github.com/shikokuchuo/secretbase/actions)
[![codecov](https://codecov.io/gh/shikokuchuo/secretbase/graph/badge.svg)](https://app.codecov.io/gh/shikokuchuo/secretbase)
[![DOI](https://zenodo.org/badge/745691432.svg)](https://zenodo.org/doi/10.5281/zenodo.10553139)
<!-- badges: end -->

SHA-256, SHA-3 cryptographic hash and SHAKE256 extendable-output
functions (XOF).

The SHA-3 Secure Hash Standard was published by the National Institute
of Standards and Technology (NIST) in 2015 at
[doi:10.6028/NIST.FIPS.202](https://dx.doi.org/10.6028/NIST.FIPS.202).
The SHA-256 Secure Hash Standard was published by NIST in 2002 at
<https://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf>.

Fast and memory-efficient implementation using the core algorithms from
‘Mbed TLS’ under the Trusted Firmware Project
<https://www.trustedfirmware.org/projects/mbed-tls/>.

### Installation

Install the latest version from the R-releases R-universe, or CRAN:

``` r
install.packages("secretbase", repos = "https://r-releases.r-universe.dev")
```

Or the development version from the author’s R-universe:

``` r
install.packages("secretbase", repos = "https://shikokuchuo.r-universe.dev")
```

### Quick Start

`secretbase` offers the functions: `sha3()` and `sha256()`.

For `sha3()`, to use:

- SHA-3 cryptographic hash algorithm, specify ‘bits’ as `224`, `256`,
  `384` or `512`
- SHAKE256 extendable-output function (XOF), specify any other bit
  length

``` r
library(secretbase)

sha3("secret base")
#> [1] "a721d57570e7ce366adee2fccbe9770723c6e3622549c31c7cab9dbb4a795520"

sha3("secret base", convert = FALSE)
#>  [1] a7 21 d5 75 70 e7 ce 36 6a de e2 fc cb e9 77 07 23 c6 e3 62 25 49 c3 1c 7c
#> [26] ab 9d bb 4a 79 55 20

sha3("秘密の基地の中", bits = 512)
#> [1] "e30cdc73f6575c40d55b5edc8eb4f97940f5ca491640b41612e02a05f3e59dd9c6c33f601d8d7a8e2ca0504b8c22f7bc69fa8f10d7c01aab392781ff4ae1e610"
```

Hash arbitrary R objects:

- uses memory-efficient ‘streaming’ serialization (no allocation of
  serialized object)
- portable as always uses R serialization v3 XDR, skipping headers
  (containing R version and encoding information)

``` r
sha3(data.frame(a = 1, b = 2), bits = 160)
#> [1] "bc5a411f87ef083296c60d6557f189b62ff9e7e6"

sha3(NULL)
#> [1] "b3e37e4c5def1bfb2841b79ef8503b83d1fed46836b5b913d7c16de92966dcee"
```

Hash files:

- in a streaming fashion, accepting files larger than memory

``` r
file <- tempfile(); cat("secret base", file = file)
sha3(file = file)
#> [1] "a721d57570e7ce366adee2fccbe9770723c6e3622549c31c7cab9dbb4a795520"
```

Hash to integer:

- specify ‘convert’ as `NA` (and ‘bits’ as `32` for a single integer
  value)
- may be supplied as deterministic random seeds for R’s pseudo random
  number generators (RNGs)

``` r
sha3("秘密の基地の中", bits = 384, convert = NA)
#>  [1]  1421990570   338241144  1760362273 -1213241427  1313032644 -1154474231
#>  [7]  1041052480   697347630 -1488396834  -917712316  1835427495  2044829552

sha3("秘密の基地の中", bits = 32, convert = NA)
#> [1] 2000208511
```

For use in parallel computing, this is a valid method for reducing to a
negligible probability that RNGs in each process may overlap. This may
be especially suitable when first-best alternatives such as using
recursive streams are too expensive or unable to preserve
reproducibility. <sup>\[1\]</sup>

### References

\[1\] Pierre L’Ecuyer, David Munger, Boris Oreshkin and Richard Simard
(2017), *“Random numbers for parallel computers: Requirements and
methods, with emphasis on GPUs”*, Mathematics and Computers in
Simulation, Vol. 135, May 2017, pp. 3-17
[doi:10.1016/j.matcom.2016.05.00](https://doi.org/10.1016/j.matcom.2016.05.005).

### Links

Links:

◈ secretbase R package: <https://shikokuchuo.net/secretbase/>

Mbed TLS website: <https://www.trustedfirmware.org/projects/mbed-tls/>

–

Please note that this project is released with a [Contributor Code of
Conduct](https://shikokuchuo.net/secretbase/CODE_OF_CONDUCT.html). By
participating in this project you agree to abide by its terms.
