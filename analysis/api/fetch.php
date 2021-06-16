<?php
header('Access-Control-Allow-Origin: *'); // allow call from vuejs

$query = $_GET['object'] ?? 'searches';

$sqlite = new SQLite3(dirname(__DIR__) . "/analyzed.sqlite");
if (!$sqlite) {
    throw new \RuntimeException("Couldn't open.");
}
switch ($query) {
    case 'searches':
        $results = $sqlite->query(
            "
                SELECT s.*, d.move FROM searches s 
                    INNER JOIN decisions d on d.search_id = s.id AND d.depth = 0
                ORDER BY s.depth
            "
        );
        header("Content-Type: application/json");
        $rows = [];
        while ($row = $results->fetchArray(SQLITE3_ASSOC)) {
            $rows[] = $row;
        }
        echo json_encode($rows);
        break;

    default:
        throw new \RuntimeException();
}
