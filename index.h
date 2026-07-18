#ifndef INDEX_H
#define INDEX_H

const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>NeuroSense Analytics Dashboard</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        :root {
            --bg-color: #050505;
            --panel-color: #121214;
            --border-color: #222225;
            --text-main: #f5f5f7;
            --text-sub: #86868b;
            
            --color-temp: #ff453a;
            --color-hum: #0a84ff;
            --color-lux: #ffd60a;
            --color-mq: #30d158;
            --color-sound: #bf5af2;
            --color-env: #5ac8fa;
            --color-stress: #ff9500;
        }
        html { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background-color: var(--bg-color); color: var(--text-main); margin:0; padding:0; -webkit-font-smoothing: antialiased; }
        body { padding: 30px 20px; max-width: 1400px; margin: 0 auto; display: flex; flex-direction: column; min-height: 95vh; }
        header { border-bottom: 1px solid var(--border-color); padding-bottom: 20px; margin-bottom: 30px; display: flex; justify-content: space-between; align-items: flex-start; }
        h1 { font-size: 2rem; font-weight: 600; letter-spacing: -0.02em; margin: 0; }
        .subtitle { color: var(--text-sub); font-size: 0.95rem; margin-top: 4px; }
        
        .clock-widget { text-align: right; background: var(--panel-color); border: 1px solid var(--border-color); padding: 10px 16px; border-radius: 12px; cursor: pointer; }
        .clock-time { font-size: 1.15rem; font-weight: 600; font-variant-numeric: tabular-nums; }
        .clock-date { font-size: 0.75rem; color: var(--text-sub); text-transform: uppercase; margin-top: 2px; }

        .controls { display: flex; gap: 12px; margin-bottom: 25px; }
        .btn { background: var(--panel-color); color: var(--text-main); border: 1px solid var(--border-color); padding: 10px 20px; border-radius: 20px; font-size: 0.85rem; cursor: pointer; text-decoration: none; }
        .btn:hover { background: var(--text-main); color: var(--bg-color); }
        .btn-primary { background: #0071e3; border-color: #0071e3; color: white; }

        .analytics-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(240px, 1fr)); gap: 20px; margin-bottom: 20px; }
        .calculated-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(480px, 1fr)); gap: 20px; margin-bottom: 40px; }
        @media(max-width: 600px) { .calculated-grid { grid-template-columns: 1fr; } }

        .metric-panel { background: var(--panel-color); border: 1px solid var(--border-color); border-radius: 18px; padding: 20px; display: flex; flex-direction: column; }
        
        .panel-header { display: flex; justify-content: space-between; align-items: baseline; margin-bottom: 15px; }
        .panel-header h3 { margin: 0; color: var(--text-sub); font-size: 0.8rem; font-weight: 600; text-transform: uppercase; letter-spacing: 0.05em; }
        .value-display { font-size: 1.75rem; font-weight: 500; letter-spacing: -0.01em; }
        .unit { font-size: 0.95rem; color: var(--text-sub); margin-left: 2px; }
        .raw-label { font-size: 0.75rem; color: var(--text-sub); margin-top: 2px; }
        .status-badge { font-size: 0.75rem; font-weight: 600; padding: 2px 8px; border-radius: 6px; margin-left: 10px; text-transform: uppercase; }

        .chart-container { position: relative; width: 100%; height: 130px; margin-top: auto; }
        .tall-chart-container { position: relative; width: 100%; height: 260px; margin-top: auto; }

        footer { margin-top: auto; padding-top: 25px; border-top: 1px solid var(--border-color); text-align: center; color: var(--text-sub); font-size: 0.8rem; }
        .footer-author { color: #38bdf8; font-weight: 500; margin: 4px 0 8px 0; }
        .footer-meta { display: flex; justify-content: center; gap: 15px; flex-wrap: wrap; font-size: 0.75rem; }
        .footer-meta span { background: #121214; padding: 2px 8px; border-radius: 8px; border: 1px solid #222225; }
    </style>
</head>
<body>
    <header>
        <div>
            <h1>NeuroSense Analytics Platform</h1>
            <div class="subtitle">Cognitive Environment Mapping Framework • Core Node Network</div>
        </div>
        <div class="clock-widget" onclick="toggleTimeMode()">
            <div id="live-time" class="clock-time">--:--:--</div>
            <div id="live-date" class="clock-date">--------</div>
        </div>
    </header>

    <div class="controls">
        <a href="/download" class="btn btn-primary" download="neuro_dataset.txt">Download Sensor Dataset (.txt)</a>
        <button class="btn" onclick="clearLogs()">Clear Device Flash Memory</button>
    </div>

    <div class="analytics-grid">
        <div class="metric-panel">
            <div class="panel-header">
                <h3>Temperature</h3>
                <div class="value-display"><span id="temp">--</span><span class="unit">&deg;C</span></div>
            </div>
            <div class="chart-container"><canvas id="chartTemp"></canvas></div>
        </div>

        <div class="metric-panel">
            <div class="panel-header">
                <h3>Humidity</h3>
                <div class="value-display"><span id="hum">--</span><span class="unit">%</span></div>
            </div>
            <div class="chart-container"><canvas id="chartHum"></canvas></div>
        </div>

        <div class="metric-panel">
            <div class="panel-header">
                <h3>Illuminance</h3>
                <div class="value-display"><span id="lux">--</span><span class="unit">lx</span></div>
            </div>
            <div class="chart-container"><canvas id="chartLux"></canvas></div>
        </div>

        <div class="metric-panel">
            <div class="panel-header">
                <h3>Air Quality</h3>
                <div>
                    <div class="value-display"><span id="mq_ppm">--</span><span class="unit">ppm</span></div>
                    <div class="raw-label" id="mq_raw">-- raw</div>
                </div>
            </div>
            <div class="chart-container"><canvas id="chartMq"></canvas></div>
        </div>

        <div class="metric-panel">
            <div class="panel-header">
                <h3>Noise Profile</h3>
                <div class="value-display"><span id="sound">--</span><span class="unit">p-p</span></div>
            </div>
            <div class="chart-container"><canvas id="chartSound"></canvas></div>
        </div>
    </div>

    <div class="calculated-grid">
        <div class="metric-panel">
            <div class="panel-header">
                <h3>Environmental Quality Index (EQI)</h3>
                <div class="value-display">
                    <span id="env_score">--</span><span class="unit">%</span>
                    <span id="env_status" class="status-badge" style="background:#222; color:#fff;">Calc...</span>
                </div>
            </div>
            <div class="tall-chart-container"><canvas id="chartEnv"></canvas></div>
        </div>

        <div class="metric-panel">
            <div class="panel-header">
                <h3>Thermal Heat Stress Index</h3>
                <div class="value-display">
                    <span id="stress_score">--</span><span class="unit">&deg;C</span>
                    <span id="stress_status" class="status-badge" style="background:#222; color:#fff;">Calc...</span>
                </div>
            </div>
            <div class="tall-chart-container"><canvas id="chartStress"></canvas></div>
        </div>
    </div>

    <footer>
      <b>Cognitive Environment Mapping Framework [CEMF]</b><br />
  Open Source Prototype Developed & Designed by <b>Md. Shihab Sikder</b><br />
 <small> Server: ESP32 MCU | Route: Home Network | Access: http://192.168.0.150 | PV1.0</small>
  
  
</div>
    </footer>

<script>
    function createMiniChart(canvasId, lineColor, labelTag) {
        const ctx = document.getElementById(canvasId).getContext('2d');
        return new Chart(ctx, {
            type: 'line',
            data: { labels: [], datasets: [{ label: labelTag, data: [], borderColor: lineColor, backgroundColor: 'transparent', tension: 0.3, borderWidth: 2, pointRadius: 0 }] },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    x: { display: false },
                    y: { 
                        type: 'linear',
                        display: true, 
                        grid: { color: '#1c1c1e', drawTicks: false }, 
                        ticks: { color: '#86868b', font: { size: 9 }, maxTicksLimit: 3 }
                    }
                },
                plugins: { legend: { display: false }, tooltip: { enabled: true, mode: 'index', intersect: false } }
            }
        });
    }

    const chartTemp   = createMiniChart('chartTemp', '#ff453a', 'Temp (°C)');
    const chartHum    = createMiniChart('chartHum', '#0a84ff', 'Humidity (%)');
    const chartLux    = createMiniChart('chartLux', '#ffd60a', 'Luminance (lx)');
    const chartMq     = createMiniChart('chartMq', '#30d158', 'Air Quality (PPM)');
    const chartSound  = createMiniChart('chartSound', '#bf5af2', 'Noise (p-p)');
    const chartEnv    = createMiniChart('chartEnv', '#5ac8fa', 'Env Stability (%)');
    const chartStress = createMiniChart('chartStress', '#ff9500', 'Heat Index (°C)');

    let is12HourMode = true;
    function updateClock() {
        const now = new Date();
        let hours = now.getHours();
        const minutes = String(now.getMinutes()).padStart(2, '0');
        const seconds = String(now.getSeconds()).padStart(2, '0');
        let ampm = '';
        if (is12HourMode) {
            ampm = hours >= 12 ? ' PM' : ' AM';
            hours = hours % 12; hours = hours ? hours : 12;
            hours = String(hours).padStart(2, '0');
        } else {
            hours = String(hours).padStart(2, '0');
        }
        document.getElementById('live-time').innerText = `${hours}:${minutes}:${seconds}${ampm}`;
        document.getElementById('live-date').innerText = now.toLocaleDateString('en-US', { weekday: 'short', year: 'numeric', month: 'short', day: 'numeric' });
    }
    function toggleTimeMode() { is12HourMode = !is12HourMode; updateClock(); }
    setInterval(updateClock, 1000);

    function pushChartData(chartInstance, val, timeLabel) {
        chartInstance.data.labels.push(timeLabel);
        chartInstance.data.datasets[0].data.push(val);
        if (chartInstance.data.labels.length > 25) {
            chartInstance.data.labels.shift();
            chartInstance.data.datasets[0].data.shift();
        }
        chartInstance.update('none');
    }

    // Recalibrated Calculation Engine
    function calculateMetrics(t, h, l, q, s) {
        // 1. RE-TUNED EQI BALANCING FOR INDOOR URBAN SPACES
        // Expands air limits up to 2000 ppm and scales sound attenuation up to 3500 p-p to handle real-world ambient setups gracefully.
        let mqScore = 100 - Math.min(Math.max((q - 400) / 1600 * 100, 0), 100);
        let soundScore = 100 - Math.min((s / 3500) * 100, 100);
        let luxScore = l > 30 ? 100 : (l / 30) * 100;
        
        let eqi = Math.round((mqScore * 0.5) + (soundScore * 0.3) + (luxScore * 0.2));

        const envBadg = document.getElementById('env_status');
        if(eqi > 75) { envBadg.innerText="Good"; envBadg.style.background="#30d158"; envBadg.style.color="#000"; }
        else if(eqi > 40) { envBadg.innerText="Average"; envBadg.style.background="#ffd60a"; envBadg.style.color="#000"; }
        else { envBadg.innerText="Poor"; envBadg.style.background="#ff453a"; envBadg.style.color="#fff"; }

        // 2. Heat Stress Index System
        let f = (t * 9/5) + 32;
        let heatIndexC = t; 

        if (f >= 80) {
            let hiF = -42.379 + 2.04901523*f + 10.14333127*h - 0.22475541*f*h - 0.00683783*f*f - 0.05481717*h*h + 0.00122874*f*f*h + 0.00085282*f*h*h - 0.00000199*f*f*h*h;
            heatIndexC = (hiF - 32) * 5/9;
        }

        const stBadg = document.getElementById('stress_status');
        if(heatIndexC < 32) { stBadg.innerText="Comfortable"; stBadg.style.background="#30d158"; stBadg.style.color="#000"; }
        else if(heatIndexC < 41) { stBadg.innerText="High Heat"; stBadg.style.background="#ffd60a"; stBadg.style.color="#000"; }
        else { stBadg.innerText="Extreme Danger"; stBadg.style.background="#ff453a"; stBadg.style.color="#fff"; }

        return { eqi: eqi, heatIndex: Number(heatIndexC.toFixed(1)) };
    }

    function updateData() {
        fetch('/data')
        .then(res => res.json())
        .then(data => {
            document.getElementById('temp').innerText = data.temperature.toFixed(1);
            document.getElementById('hum').innerText = data.humidity.toFixed(1);
            document.getElementById('lux').innerText = Math.round(data.lux);
            document.getElementById('mq_ppm').innerText = data.mq135_ppm.toFixed(1);
            document.getElementById('mq_raw').innerText = data.mq135_raw + " raw";
            document.getElementById('sound').innerText = Math.round(data.sound);

            let analytics = calculateMetrics(data.temperature, data.humidity, data.lux, data.mq135_ppm, data.sound);
            document.getElementById('env_score').innerText = analytics.eqi;
            document.getElementById('stress_score').innerText = analytics.heatIndex;

            const timeNow = document.getElementById('live-time').innerText.split(' ')[0];
            
            pushChartData(chartTemp, data.temperature, timeNow);
            pushChartData(chartHum, data.humidity, timeNow);
            pushChartData(chartLux, data.lux, timeNow);
            pushChartData(chartMq, data.mq135_ppm, timeNow);
            pushChartData(chartSound, data.sound, timeNow);
            
            pushChartData(chartEnv, analytics.eqi, timeNow);
            pushChartData(chartStress, analytics.heatIndex, timeNow);
        }).catch(err => console.error(err));
    }
    
    function clearLogs() { if(confirm("Flush local text dataset storage?")) { fetch('/clear').then(() => alert("Cleared.")); } }
    setInterval(updateData, 2000);
    window.onload = () => { updateData(); updateClock(); };
</script>
</body>
</html>
)rawliteral";

#endif