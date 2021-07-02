<?php
header('Access-Control-Allow-Origin: *'); // allow call from vuejs

$query = $_GET['object'] ?? 'searches';

$sqlite = new SQLite3(dirname(__DIR__) . "/analyzed.sqlite");
if (!$sqlite) {
    throw new \RuntimeException("Couldn't open.");
}

switch ($query) {
    case 'iterative_searches':
        $results = $sqlite->query('
            SELECT isrch.* FROM iterative_searches isrch
        ');
        header("Content-Type: application/json");
        header('Cache-Control: no-cache');
        $iterative_searches = retrieve_all($results);
        $stmt = $sqlite->prepare('
            SELECT depth FROM iterations WHERE iterative_search_id = :iterativeSearchId
        ');
        $iterative_search['iterations'] = [];
        $iterative_searches = array_map(function ($iterative_search) use ($stmt) {
            $iterative_search['iterations'] = [];
            $stmt->bindParam(':iterativeSearchId', $iterative_search['id'], SQLITE3_INTEGER);
            $results = $stmt->execute();
            $iterative_search['iterations'] = retrieve_first_column($results);
            $iterative_search['id'] = strval($iterative_search['id']);
            return $iterative_search;
        }, $iterative_searches);

        echo json_encode($iterative_searches);
        break;

    case 'searches':
        $iterative_search_id = $_GET['iterative_search_id'] ?? '';
        $iteration_depth = $_GET['depth'] ?? '';
        $stmt = $sqlite->prepare(
            "
                SELECT s.* 
                      , d.move, d.id as decision_id 
                FROM iterations i 
                    INNER JOIN searches s ON i.id = s.iteration_id
                    INNER JOIN decisions d on d.search_id = s.id AND d.depth = 0
                WHERE 
                      i.iterative_search_id = :iterativeSearchId AND i.depth = :depth
                ORDER BY s.depth
            "
        );
        $stmt->bindParam(':iterativeSearchId', $iterative_search_id, SQLITE3_INTEGER);
        $stmt->bindParam(':depth', $iteration_depth, SQLITE3_INTEGER);
        header("Content-Type: application/json");
        $rows = retrieve_all($stmt->execute());
        $results = array_map(function ($search): array {
            $search['id'] = (string)$search['id'];
            $search['iteration_id'] = (string)$search['iteration_id'];
            $search['decision_id'] = isset($search['decision_id']) ? (string)$search['decision_id'] : null;
            return $search;
        }, $rows);

        $json = json_encode($results);
        echo $json;
        break;

    case 'positions':
        $decision_id = $_GET['decision_id'] ?? '';
        $position_id = $_GET['position_id'] ?? '';
        $params = [];
        if (!empty($decision_id)) {
            $params['decisionId'] = $decision_id;
            $where = 'd.id = :decisionId';
        } else {
            $params['positionId'] = $position_id;
            $where = 'd.parent_position_id = :positionId';
        }
        $stmt = $sqlite->prepare(
            "
                SELECT p.*, d.move as final_move
                FROM decisions d 
                    INNER JOIN positions p on d.id = p.decision_id
                WHERE
                    $where
                ORDER BY p.score DESC
            "
        );
        foreach ($params as $key => $param) {
            $stmt->bindValue($key, $param, SQLITE3_TEXT);
        }
        $result = $stmt->execute();
        $results = retrieve_all($result);
        header("Content-Type: application/json");
        $results = array_map(function($position) : array {
            $position['id'] = (string)$position['id'];
            $position['decision_id'] = isset($position['decision_id']) ? (string)$position['decision_id'] : null;
            return $position;
        }, $results);
        echo json_encode($results);
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

function retrieve_first_column($results) {
    $rows = [];
    while ($row = $results->fetchArray(SQLITE3_NUM)) {
        $rows[] = $row[0];
    }
    return $rows;
}