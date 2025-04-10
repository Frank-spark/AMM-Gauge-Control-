const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Gauge Control Configuration</title>
    <script src="https://cdn.tailwindcss.com"></script>
</head>
<body class="bg-gray-100 font-sans leading-normal tracking-normal">
    <div class="container mx-auto px-4 py-8">
        <div class="bg-white shadow-md rounded px-8 pt-6 pb-8 mb-4">
            <h1 class="text-2xl font-bold text-center text-gray-700 mb-6">Gauge Control Configuration</h1>
            
            <form id="configForm" class="space-y-6">
                <div class="grid grid-cols-1 md:grid-cols-2 gap-6">
                    <!-- Resistance Settings -->
                    <div class="space-y-4">
                        <h2 class="text-xl font-semibold text-gray-600">Resistance Settings</h2>
                        
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="maxResistance">
                                Max Resistance (Empty Tank):
                            </label>
                            <div class="flex items-center">
                                <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="maxResistance" type="number" min="0" max="1000">
                                <span class="ml-2 text-gray-600">Ohms</span>
                            </div>
                        </div>
                        
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="minResistance">
                                Min Resistance (Full Tank):
                            </label>
                            <div class="flex items-center">
                                <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="minResistance" type="number" min="0" max="1000">
                                <span class="ml-2 text-gray-600">Ohms</span>
                            </div>
                        </div>
                        
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="knownResistor">
                                Known Resistor:
                            </label>
                            <div class="flex items-center">
                                <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="knownResistor" type="number" min="0" max="10000">
                                <span class="ml-2 text-gray-600">Ohms</span>
                            </div>
                        </div>
                    </div>
                    
                    <!-- PWM Settings -->
                    <div class="space-y-4">
                        <h2 class="text-xl font-semibold text-gray-600">PWM Settings</h2>
                        
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="minDuty">
                                Min Duty (Empty):
                            </label>
                            <div class="flex items-center">
                                <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="minDuty" type="number" min="0" max="255">
                            </div>
                        </div>
                        
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="maxDuty">
                                Max Duty (Full):
                            </label>
                            <div class="flex items-center">
                                <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="maxDuty" type="number" min="0" max="255">
                            </div>
                        </div>
                        
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="blinkThreshold">
                                Blink Threshold:
                            </label>
                            <div class="flex items-center">
                                <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="blinkThreshold" type="number" min="0" max="255">
                            </div>
                        </div>
                        
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="lowLevelThreshold">
                                Low Level Threshold:
                            </label>
                            <div class="flex items-center">
                                <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="lowLevelThreshold" type="number" min="0" max="255">
                            </div>
                        </div>
                    </div>
                </div>
                
                <!-- Timing Settings -->
                <div class="space-y-4">
                    <h2 class="text-xl font-semibold text-gray-600">Timing Settings</h2>
                    <div class="grid grid-cols-1 md:grid-cols-3 gap-4">
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="blinkInterval">
                                Blink Interval:
                            </label>
                            <div class="flex items-center">
                                <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="blinkInterval" type="number" min="50" max="1000">
                                <span class="ml-2 text-gray-600">ms</span>
                            </div>
                        </div>
                        
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="numSamples">
                                ADC Samples:
                            </label>
                            <div class="flex items-center">
                                <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="numSamples" type="number" min="1" max="100">
                            </div>
                        </div>
                        
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="stepDelay">
                                Step Delay:
                            </label>
                            <div class="flex items-center">
                                <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="stepDelay" type="number" min="1" max="100">
                                <span class="ml-2 text-gray-600">ms</span>
                            </div>
                        </div>
                    </div>
                </div>
                
                <!-- PEM Startup Settings -->
                <div class="space-y-4">
                    <h2 class="text-xl font-semibold text-gray-600">PEM Startup Settings</h2>
                    <div class="grid grid-cols-1 md:grid-cols-3 gap-4">
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="pemMaxValue">
                                Ramp Max Value:
                            </label>
                            <div class="flex items-center">
                                <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="pemMaxValue" type="number" min="0" max="255">
                                <span class="ml-2 text-gray-600">0-255</span>
                            </div>
                        </div>
                        
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="pemStepSize">
                                Ramp Step Size:
                            </label>
                            <div class="flex items-center">
                                <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="pemStepSize" type="number" min="1" max="50">
                                <span class="ml-2 text-gray-600">per step</span>
                            </div>
                        </div>
                        
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="stepDelay">
                                Step Delay:
                            </label>
                            <div class="flex items-center">
                                <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="stepDelay" type="number" min="1" max="100">
                                <span class="ml-2 text-gray-600">ms</span>
                            </div>
                        </div>
                    </div>
                </div>
                
                <!-- WiFi Settings -->
                <div class="space-y-4">
                    <h2 class="text-xl font-semibold text-gray-600">WiFi Settings</h2>
                    <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="wifiSSID">
                                WiFi SSID:
                            </label>
                            <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="wifiSSID" type="text">
                        </div>
                        
                        <div>
                            <label class="block text-gray-700 text-sm font-bold mb-2" for="wifiPassword">
                                WiFi Password:
                            </label>
                            <input class="shadow appearance-none border rounded w-full py-2 px-3 text-gray-700 leading-tight focus:outline-none focus:shadow-outline" id="wifiPassword" type="password">
                        </div>
                    </div>
                </div>
                
                <!-- Action Buttons -->
                <div class="flex items-center justify-between pt-4">
                    <button id="saveBtn" class="bg-blue-500 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded focus:outline-none focus:shadow-outline" type="button">
                        Save Configuration
                    </button>
                    <button id="resetBtn" class="bg-red-500 hover:bg-red-700 text-white font-bold py-2 px-4 rounded focus:outline-none focus:shadow-outline" type="button">
                        Reset to Defaults
                    </button>
                </div>
                
                <div id="statusMessage" class="mt-4 text-center hidden"></div>
            </form>
            
            <!-- Gauge Values Monitor -->
            <div class="mt-8 pt-6 border-t border-gray-200">
                <h2 class="text-xl font-semibold text-gray-600 mb-4">Live Monitor</h2>
                <div id="liveData" class="grid grid-cols-2 md:grid-cols-4 gap-4 text-center">
                    <div class="p-3 bg-gray-100 rounded shadow-inner">
                        <div class="text-sm text-gray-600">Resistance</div>
                        <div id="liveResistance" class="font-bold text-lg">-- Ω</div>
                    </div>
                    <div class="p-3 bg-gray-100 rounded shadow-inner">
                        <div class="text-sm text-gray-600">Target Duty</div>
                        <div id="liveTargetDuty" class="font-bold text-lg">--</div>
                    </div>
                    <div class="p-3 bg-gray-100 rounded shadow-inner">
                        <div class="text-sm text-gray-600">Smoothed Duty</div>
                        <div id="liveSmoothedDuty" class="font-bold text-lg">--</div>
                    </div>
                    <div class="p-3 bg-gray-100 rounded shadow-inner">
                        <div class="text-sm text-gray-600">PEM Status</div>
                        <div id="livePemStatus" class="font-bold text-lg">--</div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <script>
        // Get current configuration
        fetch('/config')
            .then(response => response.json())
            .then(config => {
                document.getElementById('maxResistance').value = config.maxResistance;
                document.getElementById('minResistance').value = config.minResistance;
                document.getElementById('knownResistor').value = config.knownResistor;
                document.getElementById('minDuty').value = config.minDuty;
                document.getElementById('maxDuty').value = config.maxDuty;
                document.getElementById('blinkThreshold').value = config.blinkThreshold;
                document.getElementById('lowLevelThreshold').value = config.lowLevelThreshold;
                document.getElementById('blinkInterval').value = config.blinkInterval;
                document.getElementById('numSamples').value = config.numSamples;
                document.getElementById('stepDelay').value = config.stepDelay;
                document.getElementById('wifiSSID').value = config.wifiSSID || '';
                document.getElementById('wifiPassword').value = config.wifiPassword || '';
                document.getElementById('pemMaxValue').value = config.pemMaxValue;
                document.getElementById('pemStepSize').value = config.pemStepSize;
            })
            .catch(error => {
                console.error('Error fetching configuration:', error);
                showStatus('Failed to load configuration', 'error');
            });

        // Save configuration
        document.getElementById('saveBtn').addEventListener('click', function() {
            const config = {
                maxResistance: parseInt(document.getElementById('maxResistance').value),
                minResistance: parseInt(document.getElementById('minResistance').value),
                knownResistor: parseInt(document.getElementById('knownResistor').value),
                minDuty: parseInt(document.getElementById('minDuty').value),
                maxDuty: parseInt(document.getElementById('maxDuty').value),
                blinkThreshold: parseInt(document.getElementById('blinkThreshold').value),
                lowLevelThreshold: parseInt(document.getElementById('lowLevelThreshold').value),
                blinkInterval: parseInt(document.getElementById('blinkInterval').value),
                numSamples: parseInt(document.getElementById('numSamples').value),
                stepDelay: parseInt(document.getElementById('stepDelay').value),
                wifiSSID: document.getElementById('wifiSSID').value,
                wifiPassword: document.getElementById('wifiPassword').value,
                pemMaxValue: parseInt(document.getElementById('pemMaxValue').value),
                pemStepSize: parseInt(document.getElementById('pemStepSize').value)
            };

            fetch('/config', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(config),
            })
            .then(response => response.json())
            .then(data => {
                showStatus('Configuration saved successfully!', 'success');
            })
            .catch(error => {
                console.error('Error saving configuration:', error);
                showStatus('Failed to save configuration', 'error');
            });
        });

        // Reset to defaults
        document.getElementById('resetBtn').addEventListener('click', function() {
            if (confirm('Are you sure you want to reset to default settings?')) {
                fetch('/reset', { method: 'POST' })
                    .then(response => response.json())
                    .then(config => {
                        document.getElementById('maxResistance').value = config.maxResistance;
                        document.getElementById('minResistance').value = config.minResistance;
                        document.getElementById('knownResistor').value = config.knownResistor;
                        document.getElementById('minDuty').value = config.minDuty;
                        document.getElementById('maxDuty').value = config.maxDuty;
                        document.getElementById('blinkThreshold').value = config.blinkThreshold;
                        document.getElementById('lowLevelThreshold').value = config.lowLevelThreshold;
                        document.getElementById('blinkInterval').value = config.blinkInterval;
                        document.getElementById('numSamples').value = config.numSamples;
                        document.getElementById('stepDelay').value = config.stepDelay;
                        document.getElementById('wifiSSID').value = config.wifiSSID || '';
                        document.getElementById('wifiPassword').value = config.wifiPassword || '';
                        document.getElementById('pemMaxValue').value = config.pemMaxValue;
                        document.getElementById('pemStepSize').value = config.pemStepSize;
                        
                        showStatus('Reset to default values', 'success');
                    })
                    .catch(error => {
                        console.error('Error resetting configuration:', error);
                        showStatus('Failed to reset configuration', 'error');
                    });
            }
        });

        // Fetch live data every 2 seconds
        setInterval(() => {
            fetch('/livedata')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('liveResistance').textContent = data.resistance.toFixed(1) + ' Ω';
                    document.getElementById('liveTargetDuty').textContent = data.targetDuty;
                    document.getElementById('liveSmoothedDuty').textContent = data.smoothedDuty;
                    document.getElementById('livePemStatus').textContent = data.pemRunning ? 'Running' : 'Stopped';
                    
                    // Add highlight class based on status
                    document.getElementById('livePemStatus').className = 
                        data.pemRunning ? 'font-bold text-lg text-green-600' : 'font-bold text-lg text-red-600';
                })
                .catch(error => {
                    console.error('Error fetching live data:', error);
                });
        }, 2000);

        function showStatus(message, type) {
            const statusEl = document.getElementById('statusMessage');
            statusEl.textContent = message;
            statusEl.className = 'mt-4 text-center p-2 rounded';
            
            if (type === 'error') {
                statusEl.classList.add('bg-red-100', 'text-red-700');
            } else {
                statusEl.classList.add('bg-green-100', 'text-green-700');
            }
            
            statusEl.classList.remove('hidden');
            
            setTimeout(() => {
                statusEl.classList.add('hidden');
            }, 3000);
        }
    </script>
</body>
</html>
)rawliteral"; 