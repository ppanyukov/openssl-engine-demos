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

// This is the main file for golang to make a .so shared lib.
//
// A couple of requirements:
//   - must be main package
//   - must import "C"
//   - must have empty main() function.
//
// It should be built with:
//    go build -buildmode=c-shared
//
// Sadly the above does not produce file ending in .so
// thus also use '-o libwhatever.so'
//
// Looks like any function which needs to be used in C code as a
// callback must be exported which is annoying. Maybe I'm wrong.
// To do so, use special //export comment (note absence of space
// between // and export!).
//
// Any interaction with native C code and libraries are via C., e.g.
//   C.whatever()
//
// The engine.c file will be compiled by go automatically
// and linked into the final executable. No need to do anything
// to make this happen.
package main

import "fmt"

// Some settings can be controlled via CFLAGS and LDFLAGS.
// Probably best have as much of it here as possible.

/*
#cgo CFLAGS: -fPIC
#cgo LDFLAGS: -lcrypto
#include <stdlib.h>
#include <openssl/engine.h>

// This is implemented in engine.c
extern void called_from_golang();

*/
import "C"

// Export any functions which are to be used from engine.c.
// Any parameters obviously need to be C-compatible.
//
// Note that there is no 'const' construct in golang, which
// might cause some compiler warnings. Cast from 'const' to
// non-const in C code to avoid these when/if required.
//
// Any functions to be called from C need to be forward-declared
// in C code.
//
// The following function is a direct golang version of
// rsa_sign function from openssl.

//export rsa_sign_go_callback
func rsa_sign_go_callback(
	xtype C.int,
	m *C.uchar, // digest to sign
	m_length C.uint, // length of digest
	sigret *C.uchar, // buffer to populate with signature
	siglen *C.uint, // the number of bytes written to sigret
	rsa *C.RSA) int {

	defer fmt.Printf("rsa_sign_go_callback: OUT\n")

	fmt.Printf("rsa_sign_go_callback: IN\n")
	fmt.Printf("rsa_sign_go_callback: xtype = %d\n", xtype)
	fmt.Printf("rsa_sign_go_callback: m_length = %d\n", m_length)
	fmt.Printf("rsa_sign_go_callback: sigret = %d\n", sigret)
	fmt.Printf("rsa_sign_go_callback: siglen = %d\n", siglen)
	fmt.Printf("rsa_sign_go_callback: sizeof(rsa) = %d\n", C.sizeof_RSA)

	C.called_from_golang()

	return 1
}

func main() {
}
