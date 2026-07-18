import os
import io
import csv
import pandas as pd
import numpy as np
import pickle
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from flask import Flask, render_template, request, jsonify, redirect, url_for

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = 'static/charts'
os.makedirs(app.config['UPLOAD_FOLDER'], exist_ok=True)

# Load Trained ML Pickles safely
with open('models/ann_model.pkl', 'rb') as f:
    ann_model = pickle.load(f)
with open('models/regression_model.pkl', 'rb') as f:
    regressor_model = pickle.load(f)

MEDICAL_DEFAULTS = {
    'temperature': {'min_ideal': 22.0, 'max_ideal': 26.0, 'unit': '°C', 'warning_high': 32.0},
    'humidity': {'min_ideal': 45.0, 'max_ideal': 65.0, 'unit': '%', 'warning_high': 80.0},
    'lux': {'min_ideal': 150.0, 'max_ideal': 500.0, 'unit': 'lx', 'warning_high': 1000.0},
    'mq135_ppm': {'min_ideal': 350.0, 'max_ideal': 700.0, 'unit': 'ppm', 'warning_high': 1000.0},
    'sound': {'min_ideal': 200.0, 'max_ideal': 600.0, 'unit': 'p-p', 'warning_high': 1500.0}
}

VULNERABILITY_PROFILES = {
    1: "Ecosystem equilibrium displays degradation trends. Deviations map directly to structural fluctuations in internal parameters.",
    2: "Homeostasis tracking warns that keeping these levels long-term shifts biological thermal and perspiration comfort boundaries.",
    3: "Cognitive payload testing indicates background noise or lighting imbalances induce minor target executive processing lag.",
    4: "Workplace efficiency models indicate atmospheric anomalies run risk of generating early structural performance drop-offs.",
    5: "Critical tracking arrays show sustained exposure patterns introduce noticeable variations away from medical baselines.",
    6: "Elevated carbon density risks developing localized cognitive fatigue, drowsiness, and executive function delays.",
    7: "Thermal index shifts put stress on core metabolic regulation channels under long-term active usage profiles.",
    8: "High continuous peak noise profile triggers heightened baseline sympathetic nervous system activation cycles.",
    9: "Sub-optimal illumination values can cause eyestrain, visual acuity strain, and localized spatial orientation delays."
}

def compute_detailed_indices(t, h, l, q, s, eqi_pred):
    if hasattr(eqi_pred, "item"):
        eqi_pred = eqi_pred.item()
    elif isinstance(eqi_pred, np.ndarray):
        eqi_pred = float(eqi_pred[0])

    comfort_idx = np.clip(100 - (abs(t - 24) * 5 + abs(h - 55) * 0.5), 0, 100)
    learning_suitability = np.clip(100 - (abs(t - 23) * 6 + np.where(s > 1200, 30, s/50) + np.where(l < 200, 20, 0)), 0, 100)
    productivity_idx = np.clip(100 - (abs(t - 22) * 4 + np.where(q > 1000, 25, 0) + np.where(s > 1500, 20, 0)), 0, 100)
    aqi_idx = np.clip((q / 2000) * 100, 0, 100)
    health_risk = np.clip((aqi_idx * 0.5) + (np.where(t > 34, 30, 0)) + (np.where(s > 2500, 20, 0)), 0, 100)
    thermal_comfort = np.clip(100 - (abs(t - 24.5) * 7 + abs(h - 50) * 0.6), 0, 100)
    acoustic_comfort = np.clip(100 - (s / 35), 0, 100)
    visual_comfort = np.where(l >= 150 and l <= 600, 100, np.clip(100 - abs(l - 375)/6, 0, 100))
    
    return {
        1: {"name": "Environment Quality Score", "score": int(round(float(eqi_pred))), "color": "#5ac8fa", "metric_key": "eqi"},
        2: {"name": "Comfort Index", "score": int(round(float(np.mean(comfort_idx)))), "color": "#34c759", "metric_key": "humidity"},
        3: {"name": "Learning Suitability Index", "score": int(round(float(np.mean(learning_suitability)))), "color": "#af52de", "metric_key": "sound"},
        4: {"name": "Productivity Index", "score": int(round(float(np.mean(productivity_idx)))), "color": "#ff9500", "metric_key": "temperature"},
        5: {"name": "Health Risk Index", "score": int(round(float(np.mean(health_risk)))), "color": "#ff3b30", "metric_key": "mq135_ppm"},
        6: {"name": "Air Quality Index", "score": int(round(float(np.mean(aqi_idx)))), "color": "#0071e3", "metric_key": "mq135_ppm"},
        7: {"name": "Thermal Comfort Index", "score": int(round(float(np.mean(thermal_comfort)))), "color": "#ffcc00", "metric_key": "temperature"},
        8: {"name": "Acoustic Comfort Index", "score": int(round(float(np.mean(acoustic_comfort)))), "color": "#a2845e", "metric_key": "sound"},
        9: {"name": "Visual Comfort Index", "score": int(round(float(np.mean(visual_comfort)))), "color": "#ffd60a", "metric_key": "lux"}
    }

@app.route('/')
def index():
    row_count = 0
    if os.path.exists('data.csv'):
        try:
            row_count = len(pd.read_csv('data.csv'))
        except Exception:
            row_count = 0
    
    run_analysis = 'cached_analytics_packet' in globals()
    data_packet = cached_analytics_packet if run_analysis else None
    
    return render_template('index.html', row_count=row_count, run_analysis=run_analysis, data=data_packet, standards=MEDICAL_DEFAULTS)

@app.route('/upload', methods=['POST'])
def upload_file():
    if 'file' not in request.files:
        return jsonify({"error": "No file part identified"}), 400
    file = request.files['file']
    if file.filename == '':
        return jsonify({"error": "No file selected"}), 400

    try:
        stream = io.StringIO(file.stream.read().decode("UTF-8"), newline=None)
        csv_reader = csv.DictReader(stream)
        
        parsed_data = []
        for row in csv_reader:
            if 'Temperature_C' in row and 'Humidity_Pct' in row:
                try:
                    parsed_data.append({
                        'temperature': float(row['Temperature_C']),
                        'humidity': float(row['Humidity_Pct']),
                        'lux': float(row['Luminance_Lux']),
                        'mq135_ppm': float(row['AirQuality_PPM']),
                        'sound': float(row['Noise_PToP'])
                    })
                except (ValueError, TypeError):
                    continue

        if not parsed_data:
            return jsonify({"error": "No valid structured log packets parsed."}), 400

        df_upload = pd.DataFrame(parsed_data)
        incoming_rows = len(df_upload)
        means = df_upload.mean().to_dict()

        if os.path.exists('data.csv'):
            try:
                df_master = pd.read_csv('data.csv')
            except Exception:
                df_master = pd.DataFrame()
            df_upload['eqi_score'] = np.nan
            df_upload['state'] = np.nan
            df_combined = pd.concat([df_master, df_upload], ignore_index=True)
            df_combined.to_csv('data.csv', index=False)
        else:
            df_upload.to_csv('data.csv', index=False)

        # Primary Dual Waveform Layout Ingestion
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8), facecolor='#121214')
        ax1.set_facecolor('#121214')
        
        for col, col_color, lbl in [('temperature', '#ff453a', 'Temp (°C)'), 
                                    ('humidity', '#0a84ff', 'Humid (%)'), 
                                    ('lux', '#ffd60a', 'Lux x0.1'), 
                                    ('mq135_ppm', '#30d158', 'Air PPM x0.05'), 
                                    ('sound', '#bf5af2', 'Sound P-P x0.05')]:
            scale = 0.1 if col == 'lux' else (0.05 if col in ['mq135_ppm', 'sound'] else 1.0)
            ax1.plot(df_upload[col] * scale, color=col_color, label=lbl, linewidth=2)
            
        ax1.set_title('Real-Time Device Telemetry Multi-Stream Data Curve', color='#f5f5f7', fontsize=12)
        ax1.grid(color='#222225')
        ax1.tick_params(colors='#86868b', labelsize=9)
        ax1.legend(facecolor='#121214', edgecolor='#222225', labelcolor='#f5f5f7')

        # Horizon Health Forecast Chart
        ax2.set_facecolor('#121214')
        time_horizons = np.array([0, 2, 5, 10, 12, 24])
        
        base_features = np.array([[means['temperature'], means['humidity'], means['lux'], means['mq135_ppm'], means['sound']]])
        eqi_base = float(regressor_model.predict(base_features)[0])
        
        predictions = [eqi_base]
        for hour in [2, 5, 10, 12, 24]:
            deg_factor = 1.0 - (hour * (means['mq135_ppm']/45000 + means['sound']/55000))
            pred_features = base_features * deg_factor
            predictions.append(float(regressor_model.predict(pred_features)[0]))

        ax2.plot(time_horizons, predictions, color='#ff9500', marker='o', linestyle='-', linewidth=2.5, label='Predicted Health Index Pathway')
        ax2.fill_between(time_horizons, predictions, color='#ff9500', alpha=0.08)
        
        ax2.axhline(y=75, color='#34c759', linestyle='--', alpha=0.75, linewidth=1.5, label='Optimal Threshold Bound (Good)')
        ax2.axhline(y=45, color='#ff3b30', linestyle='--', alpha=0.75, linewidth=1.5, label='Critical Inversion Bound (Warning)')

        ax2.set_title('Neural Analytical Core 24h Impact Forecast on Human Health', color='#f5f5f7', fontsize=12)
        ax2.set_xlabel('Horizon Scale (Hours In Future)', color='#86868b')
        ax2.set_ylabel('Health Index Score %', color='#86868b')
        ax2.grid(color='#222225')
        ax2.tick_params(colors='#86868b', labelsize=9)
        ax2.set_xticks(time_horizons)
        ax2.set_ylim(0, 105)
        ax2.legend(facecolor='#121214', edgecolor='#222225', labelcolor='#f5f5f7', loc='lower left')
        
        plt.tight_layout()
        plt.savefig(os.path.join(app.config['UPLOAD_FOLDER'], 'trend_waveform.png'), dpi=200, bbox_inches='tight')
        plt.close()

        features = np.array([[means['temperature'], means['humidity'], means['lux'], means['mq135_ppm'], means['sound']]])
        state_pred = int(ann_model.predict(features)[0])
        eqi_pred = regressor_model.predict(features)

        states_map = {0: "POOR", 1: "AVERAGE", 2: "GOOD"}
        badge_colors = {0: "#ff3b30", 1: "#ff9500", 2: "#34c759"}
        indices = compute_detailed_indices(means['temperature'], means['humidity'], means['lux'], means['mq135_ppm'], means['sound'], eqi_pred)
        
        prognosis = []
        if means['mq135_ppm'] > 900:
            prognosis.append("Elevated carbon density risks developing localized cognitive fatigue, drowsiness, and executive function delays.")
        elif means['humidity'] > 80:
            prognosis.append("Sustained relative humidity disrupts human homeostasis, completely neutralizing sweat evaporation efficiency.")
        else:
            prognosis.append("No active adverse biochemical or cognitive degradation forces identified within this environment window.")

        global cached_analytics_packet
        cached_analytics_packet = {
            "means": means, "state_str": states_map[state_pred], "badge_color": badge_colors[state_pred],
            "eqi": indices[1]["score"], "indices": indices, "prognosis": prognosis
        }

        return jsonify({"rows_parsed": incoming_rows, "total_master_rows": len(pd.read_csv('data.csv'))})
    except Exception as e:
        return jsonify({"error": f"Internal compilation fault: {str(e)}"}), 500

@app.route('/results')
def get_results():
    global cached_analytics_packet
    if 'cached_analytics_packet' not in globals():
        return redirect(url_for('index'))
    return render_template('index.html', run_analysis=True, data=cached_analytics_packet, standards=MEDICAL_DEFAULTS)

@app.route('/score/<int:score_id>')
def get_score_page(score_id):
    global cached_analytics_packet
    if 'cached_analytics_packet' not in globals():
        return redirect(url_for('index'))
    
    score_data = cached_analytics_packet['indices'].get(score_id)
    if not score_data:
        return "Index identifier out of bounds.", 404
        
    m_key = score_data['metric_key']
    current_val = cached_analytics_packet['means'].get(m_key, score_data['score'])
    
    # Establish dynamic targets based on system definitions safely
    ideal_low = 20.0
    ideal_high = 70.0
    warn_high = 90.0
    
    if m_key in MEDICAL_DEFAULTS:
        ideal_low = MEDICAL_DEFAULTS[m_key]['min_ideal']
        ideal_high = MEDICAL_DEFAULTS[m_key]['max_ideal']
        warn_high = MEDICAL_DEFAULTS[m_key]['warning_high']
    
    # Generate sub-metric diagnostic line graph
    plt.figure(figsize=(10, 4.5), facecolor='#121214')
    ax = plt.axes()
    ax.set_facecolor('#121214')
    
    x_axis = np.array([0, 1, 2, 3, 4, 5])
    y_current = np.full_like(x_axis, current_val, dtype=float)
    
    plt.plot(x_axis, y_current, color=score_data['color'], linewidth=3, label=f'Current Measured State ({round(current_val, 1)})')
    
    plt.axhline(y=ideal_high, color='#34c759', linestyle='--', linewidth=1.5, label=f'Medical Ideal Upper ({ideal_high})')
    plt.axhline(y=ideal_low, color='#30d158', linestyle=':', linewidth=1.5, label=f'Medical Ideal Lower ({ideal_low})')
    plt.axhline(y=warn_high, color='#ff3b30', linestyle='--', linewidth=1.5, label=f'Risk Warning Boundary ({warn_high})')
    
    plt.title(f'Dynamic Boundary Waveform Tracking Analysis: {score_data["name"]}', color='#f5f5f7', fontsize=12)
    plt.grid(color='#222225', linestyle='-')
    ax.tick_params(colors='#86868b', labelsize=9)
    ax.get_xaxis().set_visible(False)
    
    plt.legend(facecolor='#121214', edgecolor='#222225', labelcolor='#f5f5f7', loc='upper right')
    
    sub_chart_filename = f'sub_chart_{score_id}.png'
    plt.savefig(os.path.join(app.config['UPLOAD_FOLDER'], sub_chart_filename), dpi=150, bbox_inches='tight')
    plt.close()

    vulnerability_text = VULNERABILITY_PROFILES.get(score_id, "Standard variance thresholds active.")

    return render_template('score.html', index=score_data, main_data=cached_analytics_packet, 
                           vulnerability_text=vulnerability_text, chart_file=sub_chart_filename)

if __name__ == '__main__':
    app.run(debug=True, port=5000)