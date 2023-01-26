importScripts('wasm-cmake.js')
wasmCmake().then((obj) => {
	console.log('loaded')
	let g = new obj.Game('wisdom::Human', 'wisdom::Computer');
	g.set_max_depth( 10 );
	console.log(g);
	console.log(g.get_max_depth());
})
