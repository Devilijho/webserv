<?php

$mi_variable = getenv("UPLOAD_DIR");
echo $mi_variable;

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
