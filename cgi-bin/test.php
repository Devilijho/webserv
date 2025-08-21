#!/usr/bin/php
<?php
// Simple CGI test script
echo "Content-Type: text/html\r\n\r\n";
?>
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>CGI Test - Webserv</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 2rem auto; padding: 2rem; background: #f8f9fa; }
        .info-box { background: white; padding: 2rem; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); margin-bottom: 1rem; }
        h1 { color: #2c3e50; }
        h2 { color: #34495e; }
        table { width: 100%; border-collapse: collapse; margin-top: 1rem; }
        th, td { padding: 0.5rem; text-align: left; border-bottom: 1px solid #ddd; }
        th { background: #ecf0f1; }
        .success { color: #27ae60; font-weight: bold; }
    </style>
</head>
<body>
    <div class="info-box">
        <h1>🐘 CGI Test - PHP Funcionando</h1>
        <p class="success">✅ El servidor CGI está operativo</p>
    </div>
    
    <div class="info-box">
        <h2>Información del Sistema</h2>
        <table>
            <tr><th>PHP Version</th><td><?php echo phpversion(); ?></td></tr>
            <tr><th>Fecha/Hora</th><td><?php echo date('Y-m-d H:i:s'); ?></td></tr>
            <tr><th>Request Method</th><td><?php echo $_SERVER['REQUEST_METHOD'] ?? 'N/A'; ?></td></tr>
            <tr><th>Script Name</th><td><?php echo $_SERVER['SCRIPT_FILENAME'] ?? 'N/A'; ?></td></tr>
            <tr><th>Server Software</th><td><?php echo $_SERVER['SERVER_SOFTWARE'] ?? 'Webserv/1.0'; ?></td></tr>
        </table>
    </div>
    
    <div class="info-box">
        <h2>Variables de Entorno CGI</h2>
        <table>
            <?php
            $cgi_vars = ['REQUEST_METHOD', 'QUERY_STRING', 'CONTENT_TYPE', 'CONTENT_LENGTH', 
                         'SCRIPT_FILENAME', 'SERVER_PROTOCOL', 'GATEWAY_INTERFACE'];
            foreach ($cgi_vars as $var) {
                $value = $_SERVER[$var] ?? 'No definida';
                echo "<tr><th>$var</th><td>$value</td></tr>";
            }
            ?>
        </table>
    </div>
    
    <p><a href="/">← Volver al inicio</a></p>
</body>
</html>
