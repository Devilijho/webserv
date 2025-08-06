<?php
echo "Content-Type: text/html\r\n\r\n";
echo "<h1>✅ PHP CGI Test</h1>";
echo "<p>Hello, world!</p>";

// Show request method
echo "<p><strong>Request Method:</strong> " .
	$_SERVER["REQUEST_METHOD"] .
	"</p>";

// Show query string
echo "<p><strong>Query String:</strong> " . $_SERVER["QUERY_STRING"] . "</p>";

// Show all environment variables
echo "<h2>🌍 Environment Variables</h2>";
echo "<pre>";
foreach ($_SERVER as $key => $value) {
	echo "$key = $value\n";
}
echo "</pre>";

// If POST, read the body
if ($_SERVER["REQUEST_METHOD"] === "POST") {
	echo "<h2>📥 POST Data</h2>";
	$postData = file_get_contents("php://input");
	echo "<pre>$postData</pre>";
}
?>
