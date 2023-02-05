importScripts('wisdom-chess-web.js')

wisdomChessWeb().then((obj) => {
	console.log('loaded')
	let g = new obj.WebGame(obj.Human, obj.ChessEngine);
	g.set_max_depth( 10 );
	console.log(g);
	console.log(g.get_max_depth());
})

onmessage = (e) => {
	console.log('message received');
	self.postMessage('hello');
}
