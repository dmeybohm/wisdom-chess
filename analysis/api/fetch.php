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
                SELECT s.*, d.move, d.id as decision_id 
                FROM searches s 
                    INNER JOIN decisions d on d.search_id = s.id AND d.depth = 0
                ORDER BY s.depth
            "
        );
        header("Content-Type: application/json");
        $rows = retrieve_all($results);
        echo json_encode($rows);
        break;

    case 'positions':
        $decision_id = $_GET['decision_id'] ?? '';
        $stmt = $sqlite->prepare(
            "
                SELECT p.*, d.move as final_move 
                FROM decisions d 
                    INNER JOIN positions p on d.id = p.decision_id
                WHERE
                    d.id = :decisionId
                ORDER BY p.score DESC
            "
        );
        $stmt->bindValue(':decisionId', $decision_id, SQLITE3_TEXT);
        $result = $stmt->execute();
        header("Content-Type: application/json");
        echo json_encode(retrieve_all($result));
        break;

    default:
        throw new \RuntimeException();
}

function retrieve_all($results) {
    $rows = [];
    while ($row = $results->fetchArray(SQLITE3_ASSOC)) {
        $rows[] = $row;
    }
    return $rows;
}