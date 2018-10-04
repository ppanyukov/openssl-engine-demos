/*
 * Copyright 2018 Philip Panyukov.
 *
 * Do with it whatever you like.
 * Don't blame me if something doesn't work or
 * doesn't work as expected :) 
 *
 * If you find this useful, feel free to attribute etc,
 * or drop a note to ppanyukov at googlemail.com; but
 * you don't have to.
 *
 */
#include <openssl/engine.h>
#include <openssl/pem.h>


/**********/
static const char *demo_engine_id = "cms-demo-c";
static const char *demo_engine_name = "Demo CMS Signer Engine (C)";


/* 
 * This struct is what tells openssl that we implement
 * custom RSA stuff. Signing is part of it. Each struct
 * member here is a pointer to a function. If we set
 * it to non-NULL value, openssl will use that function,
 * otherwise it will use whatever built-in default it has.
 */
static RSA_METHOD demo_rsa = {
    "cms-demo-c",  /* name */
    NULL, /* rsa_pub_enc */
    NULL, /* rsa_pub_dec */
    NULL, /* rsa_priv_enc */
    NULL, /* rsa_priv_dec */
    
    /* Can be null */
    NULL, /* rsa_mod_exp */

    /* Can be null */
    NULL, /* bn_mod_exp */

    /* called at new */
    NULL, /* rsa_init */

    /* called at free */
    NULL, /* rsa_finish */

    0,    /* flags */
    NULL, /* app_data */

    /* We are only interested in this one here */
    NULL, /* rsa_sign */
    NULL, /* rsa_verify */

    /* in newer OpenSSL versions */
    NULL /* rsa_keygen */
};

/* The actual implementation of rsa_sign. */
static int demo_rsa_sign(int type, const unsigned char *m, unsigned int m_length, unsigned char *sigret, unsigned int *siglen, const RSA *rsa) {
  printf("demo_rsa_sign: IN\n");
  printf("demo_rsa_sign: RSA_size(rsa): %d\n", RSA_size(rsa));

  /* call to external signing API would go here  */
  printf("demo_rsa_sign: possible call to external API here.\n");

  /*
   * Writing result.
   *
   * Write signature to sigret buffer.
   * It is already pre-allocated.
   * Must not write more than RSA_size(rsa).
   * Generally, correct signature size in bytes should be RSA_size(rsa) anyway.
   *
   *
   * Var m denotes the kind of digest (SHA1, SHA256, etc) that needs to be signed.
   *
   * Return 1 for success. Don't actually need to write signature
   * to the buffer for openssl to produce CMS file.
   */
  printf("demo_rsa_sign: OUT\n");
  return 1;
}

/*
 * When using `openssl csm -sign -signer cert.pem`, it insists that 
 * the `-inkey privatekey.pem` arument is provided. Even more, it
 * insists that the public key in the signing certificate matches 
 * the public key in that 'private key'. This is annoying, since
 * we will be using external API (like Key Vault or HSM) to sign
 * content and the private key is stored there, and we don't have
 * access to it.
 * 
 * So here is the workaround. Good new is, we don't have to have
 * the real private key. All we need is the public portion of
 * it.
 * 
 * This function is this workaround. It loads the public key
 * and gives it to openssl like it's a private key.
 * 
 * The public key can be extracted from certificate like so:
 * 
 *   openssl x509 -in mycert.pem -pubkey -out mycert.pub.pem
 * 
 * Then the cms command can be invoked like this:
 * 
 *   openssl cms -sign -signer mycert.pem -inkey mycert.pub.pem -keyform EGINE
 * 
 * The 'id' parameter to this function is the value of the -inkey
 * parameter, in out case the path to the public key extracted
 * from the certificate.
 *
 * The same key can be extracted from the signer certificate
 * directly here.
 * 
 * Also, the id can be anything, including a URL to whatever. 
 *
 */
static EVP_PKEY *demo_load_private_key(ENGINE *e, const char *id, UI_METHOD *ui, void *cb) {
  printf("demo_load_private_key: IN\n");
  printf("demo_load_private_key: key id: %s\n", id);

  /* Will read the key into here */
  EVP_PKEY *rckey = NULL;

  /* The file */
  BIO *in = NULL;

  /* Read the file */
  printf("demo_load_private_key: BIO_new_file\n");
  in = BIO_new_file(id, "r");
  if (in == NULL) goto err;

  /*
   * Load public key and pretend it's private.
   * Set the attributes to tell openssl key can be used
   * for signing operations.
   */
  printf("demo_load_private_key: PEM_read_bio_PUBKEY\n");
  rckey = PEM_read_bio_PUBKEY(in, NULL, NULL, NULL);
  if (rckey == NULL) goto err;
  rckey->pkey.rsa->flags |= RSA_FLAG_EXT_PKEY | RSA_FLAG_SIGN_VER;
  goto end;
  
err:
  printf("demo_load_private_key: ERROR\n");
  goto end;

end:
  if (in != NULL) {
    BIO_free(in); in = NULL;
  }

  printf("demo_load_private_key: OUT\n");
  return rckey;
}

/* 
 * This sets up the engine and registers it with
 * openssl. ENGINE_set_id and ENGINE_set_name are required,
 * we get segmentation fault without them.
 */
static int demo_bind(ENGINE *e, const char *id)
{
  int ret = 0;

  if (!ENGINE_set_id(e, demo_engine_id)) {
    fprintf(stderr, "ENGINE_set_id failed\n");
    goto end;
  }
  if (!ENGINE_set_name(e, demo_engine_name)) {
    printf("ENGINE_set_name failed\n");
    goto end;
  }

  /* 
   * Here is our engine setup, we populate our stuct.
   * We set RES_FLAG_SIGN_VER flat to tell openssl that
   * we implement rsa_sign function here.
   */
  demo_rsa.rsa_sign = demo_rsa_sign;
  demo_rsa.flags = RSA_FLAG_SIGN_VER;

  /* Optionally delegate other ops to built-in defaults */
  const RSA_METHOD *method = RSA_PKCS1_SSLeay();
  demo_rsa.rsa_mod_exp = method->rsa_mod_exp;
  demo_rsa.bn_mod_exp = method->bn_mod_exp;
  demo_rsa.rsa_pub_dec = method->rsa_pub_dec;
  demo_rsa.rsa_pub_enc = method->rsa_pub_enc;
  demo_rsa.rsa_priv_enc = method->rsa_priv_enc;
  demo_rsa.rsa_priv_dec = method->rsa_priv_dec;
  demo_rsa.init = method->init;
  demo_rsa.finish = method->finish;


  /* Tell openssl to use our engine for RSA ops */
  if (!ENGINE_set_RSA(e, &demo_rsa)) {
    printf("ENGINE_set_RSA failed\n");
    goto end;
  }

  /* Finally, tell openssl to use our fake private key loading func */
  if (!ENGINE_set_load_privkey_function(e, demo_load_private_key)) {
    printf("ENGINE_set_load_privkey_function failed\n");
    goto end;    
  }


  ret = 1;
 end:
  return ret;
}

/*
 * These macros implement two exported functions
 * which are called by openssl when it loads our
 * engine's shared library. Openssl specifically
 * looks for these. The function implemented by
 * IMPLEMENT_DYNAMIC_BIND_FN will in turn call our
 * 'bind' function where we do engine setup.
 */
IMPLEMENT_DYNAMIC_BIND_FN(demo_bind)
IMPLEMENT_DYNAMIC_CHECK_FN()
