# Whoa!

This is an example of openssl custom  engine which shows how to implement a 
minimalist engine to sign digests using an external API, such as Azure Key Vault.

This implementation is in pure C, with just minimum boilerplate to get
started.

Useful resources:

- Engine tutorial 1:
   https://www.openssl.org/blog/blog/2015/10/08/engine-building-lesson-1-a-minimum-useless-engine/

- Engine tutorial 2:
  https://www.openssl.org/blog/blog/2015/11/23/engine-building-lesson-2-an-example-md5-engine/

# Building

Generally just run `build.sh`

Supported platforms:

- Linux. 

    - Should have the following packages installed:
        - `yum -y install gcc`
        - `yum -y install openssl-devel`

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


 
