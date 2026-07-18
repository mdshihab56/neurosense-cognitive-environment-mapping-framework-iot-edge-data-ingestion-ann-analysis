document.getElementById('file-input').addEventListener('change', function(e) {
    const file = e.target.files[0];
    if (!file) return;
    
    document.getElementById('file-name-display').innerText = `Mounted Object: ${file.name}`;
    
    const formData = new FormData();
    formData.append('file', file);
    
    // Dispatch stream payload directly to our Flask file parser pipeline
    fetch('/upload', {
        method: 'POST',
        body: formData
    })
    .then(response => response.json())
    .then(data => {
        if(data.error) {
            alert(`Ingestion Fault: ${data.error}`);
            return;
        }
        
        // Expose confirmation cards and refresh badges
        document.getElementById('parsed-rows').innerText = data.rows_parsed;
        document.getElementById('total-rows').innerText = data.total_master_rows;
        document.getElementById('row-count-badge').innerText = data.total_master_rows;
        document.getElementById('upload-status-card').style.display = 'block';
    })
    .catch(err => {
        console.error("System pipeline communication breakdown: ", err);
        alert("Fatal error transferring file to server.");
    });
});