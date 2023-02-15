console.log("loading wisdom-chess-web.js")

// Depending on the build flags that one uses, different files need to be downloaded
// to load the compiled page. The right set of files will be expanded to be downloaded
// via the directive below.

function binary(url) { // Downloads a binary file and outputs it in the specified callback
	return new Promise((ok, err) => {
		var x = new XMLHttpRequest();
		x.open('GET', url, true);
		x.responseType = 'arraybuffer';
		x.onload = () => { ok(x.response); }
		x.send(null);
	});
}

function script(url) { // Downloads a script file and adds it to DOM
	return new Promise((ok, err) => {
		var s = document.createElement('script');
		s.src = url;
		s.onload = () => {
			var c = wisdomChessWeb;
			delete wisdomChessWeb;
			ok(c);
		};
		document.body.appendChild(s);
	});
}

Promise.all([
	binary('wisdom-chess-web.js'),
	binary('wisdom-chess-web.wasm'),
	binary('wisdom-chess-web.ww.js')
]).then((r) => {
	// Detour the JS code to a separate variable to avoid instantiating with 'r' array as "this" directly to avoid strict ECMAScript/Firefox GC problems that cause a leak, see https://bugzilla.mozilla.org/show_bug.cgi?id=1540101
	var js = URL.createObjectURL(new Blob([r[0]], { type: 'application/javascript' }));
	script(js).then(function(c) {
		const promise = c({
			wasm: r[1],
			$wb: URL.createObjectURL(new Blob([r[2]], { type: 'application/javascript' })),
			js: js
		});
		promise.then(module => {
			WisdomChessWeb = module
			if (afterWisdomChessModuleLoaded) {
				afterWisdomChessModuleLoaded(WisdomChessWeb)
			}
		})
	});
});

function afterWisdomChessModuleLoaded(WisdomChessWeb) {

	console.log('after wisdom chess module loaded')	;
	console.log(WisdomChessWeb)

	let g = new WisdomChessWeb.WebGame(WisdomChessWeb.Human, WisdomChessWeb.ChessEngine);
	window.wisdomChessWeb = WisdomChessWeb

	g.startWorker();

	window.startReact();
}

// Hmm, how are those workers going to notify the main thread?
let cnt = 0;
onmessage = (e) => {
	if (cnt % 10000) {
		console.log('message received', e);
	}
}

