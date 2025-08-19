<?php
if ($_SERVER["REQUEST_METHOD"] === "POST") {
	if (
		isset($_FILES["userfile"]) &&
		$_FILES["userfile"]["error"] === UPLOAD_ERR_OK
	) {
		$target_dir = "uploads/";
		$target_file = $target_dir . basename($_FILES["userfile"]["name"]);

		if (move_uploaded_file($_FILES["userfile"]["tmp_name"], $target_file)) {
			echo "File uploaded successfully.";
		} else {
			echo "Error uploading file.";
		}
	} else {
		echo "No file uploaded.";
	}
}
