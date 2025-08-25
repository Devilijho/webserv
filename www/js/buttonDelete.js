document.getElementById("clickBtn").addEventListener("click", retrieve);

function retrieve() {
	fetch(document.getElementById("name"), {
		method: "DELETE",
	});
}
