importScripts('wasm-cmake.js')
wasmCmake().then((obj) => {
	console.log('loaded')
	let f = new obj.Foo();
	f.setVal(12);
	console.log(f.getVal());
	setTimeout(() => {
		f.setVal(42);
		console.log(f.getVal());
	}, 5000);
})
