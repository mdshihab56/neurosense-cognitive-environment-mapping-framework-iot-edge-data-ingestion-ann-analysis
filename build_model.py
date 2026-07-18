import os
import pandas as pd
import numpy as np
import pickle
from sklearn.model_selection import train_test_split
from sklearn.neural_network import MLPClassifier
from sklearn.ensemble import RandomForestRegressor
from sklearn.preprocessing import StandardScaler

# 1. Generate Environmentally Calibrated Synthetic Training Set for Bangladesh Context
np.random.seed(42)
num_samples = 5000

# Generating realistic sensor distributions (Temp, Hum, Lux, Air PPM, Noise)
temperature = np.random.uniform(18, 42, num_samples)
humidity = np.random.uniform(40, 98, num_samples)
lux = np.random.uniform(0, 3000, num_samples)
mq135_ppm = np.random.uniform(300, 2500, num_samples)
sound = np.random.uniform(100, 4000, num_samples)

# Algorithmic derivation of Multi-Parametric Cognitive Indices for Training Labelling
eqi = 100 - ((mq135_ppm - 300)/22 + (sound/50) + np.where(lux < 100, 20, 0))
eqi = np.clip(eqi, 0, 100)

# Classify environment state (0: Poor, 1: Average, 2: Good)
env_state = np.where(eqi < 45, 0, np.where(eqi < 75, 1, 2))

# Compile Base Repository DataFrame
df = pd.DataFrame({
    'temperature': temperature, 'humidity': humidity, 'lux': lux,
    'mq135_ppm': mq135_ppm, 'sound': sound, 'eqi_score': eqi, 'state': env_state
})
df.to_csv('data.csv', index=False)
print("[+] Unified Central Telemetry Repository (data.csv) constructed.")

# 2. Features and Target Matrix Preparation
X = df[['temperature', 'humidity', 'lux', 'mq135_ppm', 'sound']]
y_class = df['state']
y_reg = df['eqi_score']

# Train ANN Classifier
X_train, X_test, y_train, y_test = train_test_split(X, y_class, test_size=0.2, random_state=42)
ann = MLPClassifier(hidden_layer_sizes=(16, 8), max_iter=1000, random_state=42)
ann.fit(X_train, y_train)

# Train Multi-Output Regression Engine
X_train_r, X_test_r, y_train_r, y_test_r = train_test_split(X, y_reg, test_size=0.2, random_state=42)
regressor = RandomForestRegressor(n_estimators=50, random_state=42)
regressor.fit(X_train_r, y_train_r)

# 3. Export Compiled Binaries
os.makedirs('models', exist_ok=True)
with open('models/ann_model.pkl', 'wb') as f:
    pickle.dump(ann, f)
with open('models/regression_model.pkl', 'wb') as f:
    pickle.dump(regressor, f)

print("[+] ANN Classifier & Random Forest Regressor binaries exported successfully into /models.")