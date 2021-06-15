<?php

$query = $_GET['object'] ?? 'searches';

$sqlite = new SQLite3("analyzed.sqlite");
if (!$sqlite) {
    throw new \RuntimeException("Couldn't open.");
}
switch ($query) {
    case 'searches':
        $results = $sqlite->query("SELECT * FROM searches");
        header("Content-Type: application/json");
        echo json_encode($results->fetchArray(SQLITE3_ASSOC));
        break;

    default:
        throw new \RuntimeException();
}
