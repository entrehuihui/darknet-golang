package main

// #cgo CFLAGS: -Iinclude
// #cgo LDFLAGS: ./libdarknet.a -lm -ldl
// #include"darknet.h"
// #include"mydarknet.c"
import "C"
import (
	"unsafe"
)

func main() {
	// return
	// Test(os.Args)
	TestDetector()
}

//Test ..
func Test(args []string) {
	arg := make([](*_Ctype_char), 0) //C语言char*指针创建切片
	l := len(args)
	for _, v := range args {
		char := C.CString(v)
		defer C.free(unsafe.Pointer(char)) //释放内存
		strptr := (*_Ctype_char)(unsafe.Pointer(char))
		arg = append(arg, strptr) //将char*指针加入到arg切片
	}
	C.checkFormat(C.int(l), (**_Ctype_char)(unsafe.Pointer(&arg[0]))) //即c语言的main(int argc,char**argv)
}

// TestDetector ..
func TestDetector() {
	filename := C.CString("data/dog.jpg")
	defer C.free(unsafe.Pointer(filename))
	thresh := C.float(0.5)
	hier_thresh := C.float(0.5)
	outfile := C.CString("1")
	defer C.free(unsafe.Pointer(outfile))
	fullscreen := C.int(0)
	names := C.retNames()
	alphabet := C.load_alphabet()
	net := C.retNet()
	C.testDetector(filename, thresh, hier_thresh, outfile, fullscreen, names, alphabet, net)
	// horses.jpg
	filename = C.CString("data/horses.jpg")
	C.testDetector(filename, thresh, hier_thresh, outfile, fullscreen, names, alphabet, net)

}
