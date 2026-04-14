var WeatherProvider = require('./provider.js');
var request = WeatherProvider.request;

var WundergroundProvider = function() {
    this._super.call(this);
    this.name = 'Weather Underground';
    this.id = 'wunderground';
};

WundergroundProvider.prototype = Object.create(WeatherProvider.prototype);
WundergroundProvider.prototype.constructor = WundergroundProvider;
WundergroundProvider.prototype._super = WeatherProvider;

WundergroundProvider.prototype.withWundergroundForecast = function(lat, lon, apiKey, callback, onFailure) {
    // callback(wundergroundResponse)
    var url = 'https://api.weather.com/v1/geocode/' + lat + '/' + lon + '/forecast/hourly/48hour.json?apiKey=' + apiKey + '&language=en-US';

    console.log('Requesting ' + url);

    request(
        url,
        'GET',
        function(response) {
            var weatherData;
            try {
                weatherData = JSON.parse(response);
            }
            catch (ex) {
                onFailure({ stage: 'provider_data', code: 'wu_forecast_parse_error' });
                return;
            }

            if (!weatherData || !Array.isArray(weatherData.forecasts) || weatherData.forecasts.length === 0) {
                onFailure({ stage: 'provider_data', code: 'wu_forecast_missing_fields' });
                return;
            }

            callback(weatherData.forecasts);
        },
        function(error) {
            onFailure({ stage: 'provider_data', code: 'wu_forecast_' + error.code });
        }
    );
};

WundergroundProvider.prototype.withWundergroundCurrent = function(lat, lon, apiKey, callback, onFailure) {
    // callback(wundergroundResponse)
    var url = 'https://api.weather.com/v3/wx/observations/current?language=en-US&units=e&format=json'
        + '&apiKey=' + apiKey
        + '&geocode=' + lat + ',' + lon;

    console.log('Requesting ' + url);

    request(
        url,
        'GET',
        (function(response) {
            var weatherData;
            try {
                weatherData = JSON.parse(response);
            }
            catch (ex) {
                onFailure({ stage: 'provider_data', code: 'wu_current_parse_error' });
                return;
            }

            if (!weatherData || typeof weatherData.temperature !== 'number') {
                onFailure({ stage: 'provider_data', code: 'wu_current_missing_fields' });
                return;
            }

            callback(weatherData);
        }).bind(this),
        function(error) {
            onFailure({ stage: 'provider_data', code: 'wu_current_' + error.code });
        }
    );
};

WundergroundProvider.prototype.clearApiKey = function() {
    localStorage.removeItem('wundergroundApiKey');
    console.log('Cleared API key');
};

WundergroundProvider.prototype.withApiKey = function(callback, onFailure) {
    // callback(apiKey)

    var apiKey = localStorage.getItem('wundergroundApiKey');
    var url = 'https://www.wunderground.com/';

    if (apiKey === null) {
        console.log('Fetching Weather Underground API key');

        request(
            url,
            'GET',
            function(response) {
                var match = response.match(/observations\/current\?apiKey=([a-z0-9]*)/);
                if (!match || !match[1]) {
                    onFailure({ stage: 'provider_data', code: 'wu_api_key_not_found' });
                    return;
                }

                apiKey = match[1];
                localStorage.setItem('wundergroundApiKey', apiKey);
                console.log('Fetched Weather Underground API key: ' + apiKey);
                callback(apiKey);
            },
            function(error) {
                onFailure({ stage: 'provider_data', code: 'wu_api_key_' + error.code });
            }
        );
    }
    else {
        console.log('Using saved API key for Weather Underground');
        callback(apiKey);
    }
};

WundergroundProvider.prototype.withWundergroundDailyForecast = function(lat, lon, apiKey, callback, onFailure) {
    // V1 10-day is often more accessible than V3 daily
    var url = 'https://api.weather.com/v1/geocode/' + lat + '/' + lon + '/forecast/daily/10day.json?language=en-US&units=e&apiKey=' + apiKey;

    console.log('Requesting V1 daily: ' + url);

    request(
        url,
        'GET',
        function(response) {
            var weatherData;
            try {
                weatherData = JSON.parse(response);
            }
            catch (ex) {
                console.log('V1 daily parse fail');
                callback([0, 0, 0, 0, 0, 0, 0]);
                return;
            }

            if (!weatherData || !Array.isArray(weatherData.forecasts)) {
                console.log('V1 daily missing forecasts');
                callback([0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0]);
                return;
            }

            // Map the next 7 days
            var dailyPrecip = weatherData.forecasts.slice(0, 7).map(function(day) {
                // Return max of day/night pop
                var dayPop = (day.day && typeof day.day.pop === 'number') ? day.day.pop : 0;
                var nightPop = (day.night && typeof day.night.pop === 'number') ? day.night.pop : 0;
                return Math.max(dayPop, nightPop);
            });
            var dailyHi = weatherData.forecasts.slice(0, 7).map(function(day) {
                return typeof day.max_temp === 'number' ? day.max_temp : 0;
            });
            var dailyLo = weatherData.forecasts.slice(0, 7).map(function(day) {
                return typeof day.min_temp === 'number' ? day.min_temp : 0;
            });
            // Pad if short
            while (dailyPrecip.length < 7) { dailyPrecip.push(0); dailyHi.push(0); dailyLo.push(0); }

            callback(dailyPrecip, dailyHi, dailyLo);
        },
        function(error) {
            console.log('Daily fetch failed, using zero bars');
            callback([0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0]);
        }
    );
};

// ============== IMPORTANT OVERRIDE ================

WundergroundProvider.prototype.withProviderData = function(lat, lon, force, onSuccess, onFailure) {
    // onSuccess expects that this.hasValidData() will be true

    if (force) {
        // In case the API key becomes invalid
        console.log('Clearing Weather Underground API key for forced update');
        this.clearApiKey();
    }

    this.withApiKey((function(apiKey) {
        this.withWundergroundCurrent(lat, lon, apiKey, (function(currentData) {
            this.withWundergroundForecast(lat, lon, apiKey, (function(forecast) {
                this.withWundergroundDailyForecast(lat, lon, apiKey, (function(dailyPrecip, temp7dayHi, temp7dayLo) {
                    this.tempTrend = forecast.map(function(entry) {
                        return entry.temp;
                    });
                    this.precipTrend = forecast.map(function(entry) {
                        return entry.pop / 100.0;
                    });
                    this.startTime = forecast[0].fcst_valid;
                    this.currentTemp = currentData.temperature;
                    this.humidity = currentData.relativeHumidity;
                    this.windSpeed = currentData.windSpeed;
                    this.windDeg = currentData.windDirection;
                    // Try to find gust in current observation, otherwise fallback to speed
                    this.windGust = currentData.windGust || currentData.gust || currentData.windSpeed;
                    this.pressure = currentData.pressureAltimeter || currentData.pressureMeanSeaLevel || 29.92;
                    this.precip7day = dailyPrecip;
                    this.temp7dayHi = temp7dayHi;
                    this.temp7dayLo = temp7dayLo;
                    onSuccess();
                }).bind(this), onFailure);
            }).bind(this), onFailure);
        }).bind(this), onFailure);
    }).bind(this), onFailure);
};

module.exports = WundergroundProvider;
