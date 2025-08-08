<?php
echo "PHP CGI Test";

echo "Request Method: " . $_SERVER["REQUEST_METHOD"] . "\n";

echo "Query String: " . $_SERVER["QUERY_STRING"] . "\n";

echo "🌍 Environment Variables\n";

foreach ($_SERVER as $key => $value) {
	echo "$key = $value\n";
}

echo "\nREQUEST METHOD TEST\n";

// if ($_SERVER["REQUEST_METHOD"] === "POST") {
// 	echo "<h2> POST Data</h2>";
// 	$postData = file_get_contents("php://input");
// 	echo "<pre>$postData</pre>\n";
// }

?>
