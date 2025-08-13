<?php
if (isset($_GET["age"]) && isset($_GET["name"])) {
	echo "<br>Hello " . $_GET["name"] . "!";
	if ($_GET["age"] >= 20) {
		echo " You are old";
	} else {
		echo " You are young";
	}
} else {
	echo "<br>Please provide a name and age";
}
?>
