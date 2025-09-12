document.getElementById("clickBtn").addEventListener("click", retrieve);

function retrieve() {
	const url = "http://localhost:8080/upload/";
	const file = url + document.getElementById("name").value;
	const xhr = new XMLHttpRequest();
	xhr.open("DELETE", file);
	xhr.send();
	// alert("Deleting file in->" + file);
	// fetch(file, {
	// 	method: "DELETE"
	// });
}
