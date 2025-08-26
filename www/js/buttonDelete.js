document.getElementById("clickBtn").addEventListener("click", retrieve);

function retrieve() {
	alert("Deleting file named " + document.getElementById("name").value);
	fetch("../uploads/" + document.getElementById("name").value, {
		method: "DELETE",
	});
}
