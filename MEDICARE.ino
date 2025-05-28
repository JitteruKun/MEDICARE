<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>MEDICARE Dashboard</title>
  <link rel="stylesheet" href="styles.css" />
  <link href="https://fonts.googleapis.com/icon?family=Material+Icons" rel="stylesheet" />
  <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Material+Symbols+Outlined" />
</head>

<body>
  <div class="sidebar">
    <div class="logo">
      <span class="material-symbols-outlined">favorite</span>
      <h2>MEDICARE</h2>
    </div>
    <nav>
      <a href="#">Overview</a>
      <a href="#">Patients</a>
      <a href="#">Vitals</a>
      <a href="#">Reports</a>
      <a href="#">Settings</a>
    </nav>
    <button class="logout">Log Out</button>
  </div>

  <div class="dashboard">
    <header>
      <h1>Dashboard</h1>
      <div class="search-bar">
        <input type="text" placeholder="Search patients..." />
      </div>
    </header>

    <div class="stats">
      <div class="card heart-rate">
        <h3>Heart Rate</h3>
        <p>72 bpm</p>
      </div>
      <div class="card oxygen-level">
        <h3>Oxygen Level</h3>
        <p>98%</p>
      </div>
      <div class="card body-temp">
        <h3>Body Temp</h3>
        <p>36.6°C</p>
      </div>
      <div class="card alerts">
        <h3>Alerts</h3>
        <p>2</p>
      </div>
    </div>

    <div class="charts">
      <div id="lineChart"></div>
      <div id="chartdiv"></div>
      <div id="aiInsightsBox" class="ai-box">
        <h3>AI Insights</h3>
        <p id="aiResponse">Loading insights...</p>
      </div>
    </div>
    

    <section class="support">
      <h2>Support Tickets</h2>
      <ul>
        <li>ECG Anomaly Detected - Resolved</li>
        <li>Oxygen Drop Alert - Pending</li>
      </ul>
    </section>
  </div>

  <!-- Chart Libraries -->
  <script src="https://cdn.amcharts.com/lib/4/core.js"></script>
  <script src="https://cdn.amcharts.com/lib/4/charts.js"></script>
  <script src="https://cdn.amcharts.com/lib/4/themes/animated.js"></script>
  <!-- Custom JS -->
  <script src="script.js"></script>
  <script src="mistral-api.js"></script>
</body>
</html>
