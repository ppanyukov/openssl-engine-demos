# Whoa!

This is same as `openssl-cms-engine`, except it implements part of
it in golang, rather than pure C. It builds up on the original
example though, and the build process is different.

The sample shows how to implement a minimalist engine to 
sign digests using an external API, such as Azure Key Vault.
And the actual signing and calls to that API is from golang.

Motivation:

1. golang is awesome, everyone knows that;
2. it's way better, simpler, and quicker to do stuff in golang than in C;
3. who wants to do libcurl and libjson in C?
4. external API may have golang SDK but not C SDK;
5. it's fun!

The example is self-contained, so no need to refer to pure-C
version.

# Golang+C

Golang does interoperate with C well. That said, how do
we make part of the engine in C and part of it in golang?

While experimenting with C+golang concept, I found a few gotchas
and limitations. With some approaches, the result seems to be
OK and working but then *boom* -- core dump or something.
Quick a few interesting linking issues depending on the frontend
in use: `gcc` vs `clang` for example. Equally, depending
on the flags supplied to golang compiler may result in good or
bad results.


## Approach 1 (selected): let go (haha) do all the work!

This is the one that is working and is working pretty well.
The sample here demostrate this approach.

The binary produced is **one** shared library (.so/.dynlib) just
like in other samples.

I found this working reliably.

The path to success mostly seem to be in two parts:

- the "correct" way to organising mixture of C and golang code;
- the "correct" way to compiling and linking.

In particular:

- put `.c` and `.go` files in the same directory;

- change into that directory and build from there like so:

    ```
    CGO_CFLAGS="-fPIC" go build -buildmode=c-shared -o libwhatever.so
    ```

- go will compile all `.c` file in the directory

- do *not* invoke gcc/clang/ld manually at any point.

- some additional parameters like include paths and library paths
  may be necessary depending on the environment.



In short: 

    > **let golang do everything for you**


Refer to `build.sh` for the nitty gritty.


## Approach 2 (discarded): two shared libraries**

```
- libengine.so (in C as per other example)
  |
  +-- loads and calls exported function(s) in libingo.so
```

This is probably most sensible, however I didn't want
**two** libraries because it just complicates things
with all those `LD_LIBRARY_PATH` and whatnot. 

Plus it's less fun!


## Approach 3 (doesn't work): use gcc, go, and ld to compile and link stuff

THIS DOES NOT WORK! Although it's not apparent why it shouldn't. This 
was the very first approach I tried and had to give up.

```
# compile C part
engine.c
   |
   +-- gcc engine.c -o engine.o

# compile golang part into static library (archive)
engine.go
   |
   +-- go build -buildmode=c-archive -o engine-go.a engine.go

# link just like everything else!
ld -lcrypto -o engine.so engine.o engine.a
```

For multiple reasons this doesn't work, especially
at the linking stage. In any case, I couldn't find the
magic combination of flags to `gcc`, `go`, and `ld` that
would produce a working result -- it either wouldn't link
at all; or some random segmentation fault would sometimes
happen at runtime.


# Building

Generally just run `build.sh`.  

Supported platforms:

- Linux. 

    - Should have the following packages installed:
        - `yum -y install epel-release  # for golang`
        - `yum -y install gcc`
        - `yum -y install openssl-devel`
        - `yum -y install golang`

    - the produced binary is ELF `engine.so`
    

- OS X. Should have:

    - gcc. Suppose use xcode?
    - openssl. Use `brew install openssl`. This usually gets installed into
      `/usr/local/opt/openssl`
    - the produced binary is OS X `engine.dylib`

- Docker/Linux.

    - build can run in docker linux container and should just work.
    - run `USE_DOCKER=true ./build.sh` to do so.
    - the produced binary is ELF `engine.so`


# Running

To use the engine.

```
# Extract public key from the signing certificate
openssl x509 -in mycert.pem -pubkey -out mycert.pub.pem
```

```
# Make cms message.
# On OS X use 'signer.dlyb' instead of 'signer.so'
openssl cms -sign -signer mycert.pem -inkey mycert.pub.pem -keyform ENGINE -engine ./engine.so
```

Note that `openssl` wants private key. And we use `-keyform ENGINE` here
to tell it we use our own function to load this private key. In actual
fact that function just needs (and returns) the public key, since the
real private key would be stored in external HSM such as Key Vault.

## NOTE ON OS X

The built-in `openssl` which comes with OS X is not suitable.
Must install another version using `brew install openssl`.
This usually gets put into `/usr/local/opt/openssl`. 

To run this version do one of the following:

- use full path, e.g `/usr/local/opt/openssl/bin/openssl`
- use alias, e.g `alias='/usr/local/opt/openssl/bin/openssl'`
- make it first on the path: `export PATH=/usr/local/opt/openssl/bin:${PATH}`


 
