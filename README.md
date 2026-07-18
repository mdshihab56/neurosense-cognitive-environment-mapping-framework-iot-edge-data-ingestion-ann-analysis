<img width="4000" height="2250" alt="NeuroSense Analytics Platform STRUCTURE_1ai" src="https://github.com/user-attachments/assets/7fee7088-9803-4886-846d-dcc812c51b3c" /><img width="4000" height="2250" alt="NeuroSense Analytics Platform STRUCTURE_2ai" src="https://github.com/user-attachments/assets/08fdebb5-4a2b-482c-8e9d-03ba4f67bac7" /># NeuroSense Cognitive Environment Mapping Framework
## IoT Edge Computing Data Ingestion Platform & Artificial Neural Network Environment State Analysis

This project is an integrated, edge-to-cloud **NeuroSense Cognitive Environment Mapping Framework** designed to track ambient atmospheric conditions and analyze their direct impact on human health and cognitive performance. The system features an array of precision hardware sensors managed by an ESP32 microcontroller, all housed within a modular, multi-tier custom enclosure constructed from transparent acrylic sheets, washers, and metallic hex standoffs. Telemetry data is smoothed locally using filtering algorithms, transmitted via a resilient Wi-Fi pipeline, and processed by a specialized Flask backend server running predictive Machine Learning models (ANN Classifier and Future Regression Forecasts) to generate real-time environmental quality indices and responsive boundary tracking dashboards.

---

## 1. Structural Architecture of Multi-Tiered NeuroSense Analytics Platform

This section details the physical engineering and spatial architecture of the prototype. The hardware framework is structured as a vertical, four-tier stack for optimal thermal isolation, electromagnetically clean signal routing, structural rigidity, and field modularity.

The structural foundation utilizes precision-machined, high-optical-clarity **3 in × 3 in transparent acrylic substrates**, providing complete component visibility for ongoing inspection while acting as an effective dielectric insulator between subsystem layers.

---

## 2. Layer-by-Layer Architecture

#### Structural Framework Parameters & Node Allocation Design
<img width="4000" height="2250" alt="NeuroSense Analytics Platform STRUCTURE_1ai" src="https://github.com/user-attachments/assets/c3214a0a-1f43-4f30-82ec-64e7dcad310e" />

#### Multi-Tiered Substrate Breakdown & Node Allocation Design
<img width="4000" height="2250" alt="NeuroSense Analytics Platform STRUCTURE_2ai" src="https://github.com/user-attachments/assets/a1094655-5d5e-4b39-9575-7c8fb648e9f2" />

#### Power, Microcontroller & Sensor Array Connection Diagram
<img width="4000" height="2250" alt="NeuroSense Analytics Platform STRUCTURE_Wiring_3ai" src="https://github.com/user-attachments/assets/96d6affa-5018-415b-8a09-85b896b76f6e" />


---

## 3. Signal Processing and Filtering

Raw analog readings from the MQ-135 and sound sensors often jump around due to minor environmental shifts or electrical noise. The firmware cleans this up using an Exponential Moving Average (EMA) filter:

```math
Y_n = \alpha \cdot X_n + (1-\alpha)\cdot Y_{n-1}
```

Where

- **Xₙ** = Raw incoming sensor reading
- **Yₙ** = Newly smoothed sensor reading
- **Yₙ₋₁** = Previous filtered value
- **α** = Smoothing factor (0.15)

This mathematical operation smooths sudden spikes while maintaining responsiveness to genuine environmental changes.

For sound processing, the microcontroller samples the microphone continuously for 100 milliseconds to capture the Peak-to-Peak voltage:

```math
V_{pp}=Sample_{max}-Sample_{min}
```

This improves ambient sound estimation by ignoring brief quiet intervals.

---

## 4. CSV Packet Generation and HTTP Transmission Core

Once the upload timer triggers, the ESP32 gathers the filtered environmental averages and packages them into a standardized comma-separated text packet.

The firmware establishes a Wi-Fi connection using the WiFi Client library and uploads the generated dataset to the Flask backend through an HTTP multipart/form-data POST request directed to the `/upload` endpoint.

---

## 5. System Folder Architecture

When deploying this project, the workspace directory must follow the structure below. This organization allows Flask to correctly locate templates, generated charts, machine learning models, and static assets.

```text
Ns-cemf/

├── app.py
├── data.csv
|
├── models/
│   ├── ann_model.pkl
│   └── regression_model.pkl
|
├── static/
│   ├── css/
│   │   └── style.css
│   │
│   └── charts/
│       ├── trend_waveform.png
│       ├── sub_chart_1.png
│       └── ...
|
└── templates/
    ├── index.html
    └── score.html
```

---

## 6. Data Scaling and Normalization

All sensor inputs are normalized using Min-Max Scaling.

```math
x_{scaled}=\frac{x-x_{min}}{x_{max}-x_{min}}
```

Reference ranges:

- Temperature Layer

```
Tmin = 0°C
Tmax = 50°C
```

25°C becomes

```
0.50
```

---

- Humidity Layer

```
Hmin = 0%
Hmax = 100%
```

55% becomes

```
0.55
```

---

- Light Layer

```
Lmin = 0 Lux
Lmax = 1000 Lux
```

350 Lux becomes

```
0.35
```

---

## 7. Matrix Multiplication in the Classifier Model

The backend contains a trained Artificial Neural Network stored inside

```
ann_model.pkl
```

The network uses weight matrices **W** and bias vectors **b** for inference.

---

## 8. Hidden Layer Processing

The normalized feature vector passes through the first hidden layer.

```math
z_{1,0}
=
(W_{1,0,0}t_{scaled})
+
(W_{1,0,1}h_{scaled})
+
(W_{1,0,2}l_{scaled})
+
b_{1,0}
```

```math
z_{1,1}
=
(W_{1,1,0}t_{scaled})
+
(W_{1,1,1}h_{scaled})
+
(W_{1,1,2}l_{scaled})
+
b_{1,1}
```

The hidden neurons then apply the ReLU activation function:

```math
a_{1,0}=max(0,z_{1,0})
```

```math
a_{1,1}=max(0,z_{1,1})
```

## 9. Output Layer Processing

The filtered hidden layer outputs are multiplied by the second layer's weight matrix and adjusted by the output bias.

```math
z_2=(W_{2,0,0}a_{1,0})+(W_{2,0,1}a_{1,1})+b_{2,0}
```

The output then passes through the Sigmoid Activation Function.

```math
\sigma(z_2)=\frac{1}{1+e^{-z_2}}
```

The sigmoid compresses the ANN prediction into a normalized value between **0** and **1**, allowing the framework to classify the current environmental condition.

### Dashboard Classification

| Sigmoid Output | Environmental State |
|----------------|---------------------|
| > 0.75 | GOOD / OPTIMAL INTERACTION |
| 0.45 – 0.75 | AVERAGE / MODERATE ADAPTATION |
| < 0.45 | POOR / CRITICAL STRESS CONDITION |

---

## 10. Regression Analysis and Time-Series Environmental Decay Math

The framework incorporates a regression forecasting engine (`regression_model.pkl`) that predicts how current environmental conditions may influence human health over the next **24 hours**.

The degradation multiplier is calculated as

```math
D_{hour}=1.0-hour\left(\frac{MeanMQ135}{45000}+\frac{MeanSound}{55000}\right)
```

Future environmental values are estimated by

```math
X_{predicted}=X_{baseline}\times D_{hour}
```

Prediction timeline

```
0 Hour
2 Hour
5 Hour
10 Hour
12 Hour
24 Hour
```

These projected values are passed into the regression model to visualize environmental degradation over time.

---

## 11. Comprehensive Environmental & Biometric Index Calculations

Beyond the ANN and Regression models, the backend computes multiple environmental quality indices.

```text
                  Input Sensor Array
        (Temperature, Humidity, Light,
          Noise, Air Quality)

                    │
                    ▼

    ┌───────────────┬───────────────┬───────────────┐
    ▼               ▼               ▼
Comfort Index  Productivity Index  Health Risk Index
```

---

### i. Comfort Index Calculation

The Comfort Index estimates thermal comfort by comparing measured values against ideal indoor conditions.

```math
Comfort=
100-
(
|t-24|\times5
+
|h-55|\times0.5
)
```

The result is clipped between **0** and **100**.

---

### ii. Learning Suitability Index Calculation

Learning Suitability evaluates how supportive the current environment is for focused cognitive work.

```math
Learning=
100-
(
|t-23|\times6
+
NoisePenalty
+
LightPenalty
)
```

Noise Penalty

```math
30,\; if\; s>1200
```

otherwise

```math
s/50
```

Light Penalty

```math
20,\; if\; l<200
```

otherwise

```math
0
```

---

### iii. Productivity Index Calculation

The Productivity Index estimates workplace efficiency using temperature, air quality, and acoustic conditions.

```math
Productivity=
100-
(
|t-22|\times4
+
AirPenalty
+
AcousticPenalty
)
```

Air Penalty

```math
25,\; if\; q>1000
```

Acoustic Penalty

```math
20,\; if\; s>1500
```

---

### iv. Air Quality and Health Risk Matrix

Air Quality Score

```math
AQI=
min
\left(
\frac{q}{2000}\times100,
100
\right)
```

Health Risk

```math
HealthRisk=
min
(
AQI\times0.5
+
HeatRisk
+
SoundRisk,
100
)
```

Heat Risk

```math
30,\; if\; Temperature>34°C
```

Sound Risk

```math
20,\; if\; Sound>2500
```

---

## 12. Generating Dashboard Visuals with Matplotlib

The backend automatically generates visual analytics after every uploaded dataset.

### Canvas Generation

Creates a high-resolution plotting canvas using a dark theme that matches the NeuroSense dashboard.

### Current State Plot

Plots the current environmental measurements as a bold baseline reference.

### Threshold Boundaries

Color-coded threshold lines represent

- 🟢 Optimal Range
- 🟡 Warning Zone
- 🔴 Critical Zone

using internationally accepted environmental guideline values.

Generated graphs include

- Temperature Trend
- Humidity Trend
- Light Intensity Trend
- Air Quality Trend
- Noise Trend
- Environmental Prediction Curve

---

## 13. The Human Impact: How This System Benefits Daily Life

NeuroSense aims to improve indoor environmental awareness by providing meaningful interpretation of sensor data rather than simply displaying raw measurements.

Potential benefits include

- Mitigation of afternoon sluggishness and tension headaches.
- Early detection of environmental conditions that may reduce cognitive efficiency.
- Reduction of chronic environmental stress factors.
- Elimination of workplace optimization guesswork using measurable data.
- Protection of long-term indoor comfort and wellness.
- Real-time optimization of executive functioning and productivity windows.
