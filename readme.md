Demos of custom engines for openssl.

See individual directories.

# Where this comes from

In undisclosed location we use HSM to store private keys.
This HSM comes with openssl engine so we can perform operations
like encrypt and sign using the HSM and keys stored in it.

The way we'd use this would be like this:

```
openssl cms -sign -signer cert.pem -engine hsm_engine -in file_to_sign.txt -out signed_file.cms
```

This would produce a digitally signed CMS-compliant file.

Then we'd go and verify this like so:

```
openssl cms -verify -in signed_file.cms
```


A small challenge popped up: Can we use Azure KeyVault instead
of this HSM? After all, KeyVault can do encryption and signing
for us just like HSM can right?

And so I went and tried to figure out how to implement a
custom engine which would use KeyVault to do signing in CMS
flow.

It turned out an interesting project! In the end, the code
is simple, but to figure out what exactly needs to be written
and in which order was not that simple and took time.

If openssl repo on github had a well-documented sample, that
would make my task so much easier! Unfortunately I couldn't
find anything suitable for a total noob like me.

This repo is result of my attempt to have this well-documented
and explained example of an engine which can be use with
`openssl cms -sign` command.

If anyone finds this helpful and it saves anyone time -- that's great!

ACKNOWLEDGEMENTS:

- Many thanks to this blog which got me started:
  https://www.openssl.org/blog/blog/2015/10/08/engine-building-lesson-1-a-minimum-useless-engine/



