<?php
echo "Content-Type: text/html\n\n";
echo "=== PHP CONFIGURATION PATHS ===\n";
echo "Configuration File Path: " . php_ini_loaded_file() . "\n";
echo "Scan directories: " . php_ini_scanned_files() . "\n";
echo "PHPRC environment: " . ($_ENV["PHPRC"] ?? "not set") . "\n";

echo "\n=== CURRENT LIMITS ===\n";
echo "upload_max_filesize: " . ini_get("upload_max_filesize") . "\n";
echo "post_max_size: " . ini_get("post_max_size") . "\n";
echo "memory_limit: " . ini_get("memory_limit") . "\n";

if ($_SERVER["REQUEST_METHOD"] === "POST") {
	echo " | POST request recieved";
	if (isset($_FILES["userfile"])) {
		echo " | File upload detected";
		if ($_FILES["userfile"]["error"] !== UPLOAD_ERR_OK) {
			switch ($_FILES["userfile"]["error"]) {
				case UPLOAD_ERR_PARTIAL:
					exit("File only partially uploaded");
				case UPLOAD_ERR_NO_FILE:
					exit("No file was uploaded");
				case UPLOAD_ERR_EXTENSION:
					exit("File upload stopped by a PHP extension");
				case UPLOAD_ERR_FORM_SIZE:
					exit("File exceeds MAX_FILE_SIZE in the HTML form");
				case UPLOAD_ERR_INI_SIZE:
					exit("File exceeds upload_max_filesize in php.ini");
				case UPLOAD_ERR_NO_TMP_DIR:
					exit("Temporary folder not found");
				case UPLOAD_ERR_CANT_WRITE:
					exit("Failed to write file");
				default:
					exit("Unknown upload error");
			}
		}
		$target_dir = "../upload/";
		$target_file = $target_dir . basename($_FILES["userfile"]["name"]);

		if (move_uploaded_file($_FILES["userfile"]["tmp_name"], $target_file)) {
			echo " | File uploaded successfully.";
		} else {
			echo " | Error uploading file.";
		}
	} else {
		echo " | userfile not set";
	}
}
?>
