<!doctype html>
<html lang="en">
<head>
    <link rel="stylesheet" href="node_modules/@chrisoakman/chessboardjs/dist/chessboard-1.0.0.css"/>
    <script src="node_modules/jquery/dist/jquery.js"></script>
    <script src="node_modules/@chrisoakman/chessboardjs/dist/chessboard-1.0.0.min.js"></script>
    <title>Chess Analyzer</title>
</head>
<body>
<div id="board1" style="width: 400px"></div>
<script>
    function searches() {
        fetch('fetch.php?object=searches').then(data => data.json())
        .then(data => {
            alert(JSON.stringify(data));
            var config = {
                position: data.fen
            }
            var board1 = Chessboard('board1', config);
        })
    }
    searches();
</script>
</body>
</html>
