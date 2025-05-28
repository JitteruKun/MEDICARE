document.addEventListener("DOMContentLoaded", function () {
  am4core.ready(function () {
    am4core.useTheme(am4themes_animated);

    // --- LINE CHART ---
    var lineChart = am4core.create("lineChart", am4charts.XYChart);
    lineChart.paddingRight = 40;
    lineChart.paddingLeft = 10;
    lineChart.interpolationDuration = 200;
    lineChart.sequencedInterpolation = false;

    var data = [];
    var now = new Date();

    for (let i = -100; i <= 0; i++) {
      let time = new Date(now.getTime() + i * 200);
      data.push({
        date: time,
        heart: 72 + Math.random() * 1.5,
        oxygen: 97 + Math.random() * 1.5,
        temp: 36.5 + Math.random() * 0.2,
      });
    }

    lineChart.data = data;

    var dateAxis = lineChart.xAxes.push(new am4charts.DateAxis());
    dateAxis.baseInterval = { timeUnit: "millisecond", count: 200 };
    dateAxis.renderer.grid.template.disabled = true;
    dateAxis.renderer.labels.template.disabled = true;
    dateAxis.tooltip.disabled = true;

    var valueAxis = lineChart.yAxes.push(new am4charts.ValueAxis());
    valueAxis.min = 20;
    valueAxis.max = 100;
    valueAxis.strictMinMax = true;
    valueAxis.renderer.grid.template.strokeOpacity = 0.15;
    valueAxis.renderer.minGridDistance = 50;

    function createSeries(field, name, color) {
      var series = lineChart.series.push(new am4charts.LineSeries());
      series.dataFields.dateX = "date";
      series.dataFields.valueY = field;
      series.stroke = am4core.color(color);
      series.strokeWidth = 2;
      series.name = name;
      series.tensionX = 0.9;

      var bullet = series.bullets.push(new am4charts.CircleBullet());
      bullet.circle.radius = 3;
      bullet.fill = am4core.color(color);
      bullet.strokeWidth = 0;

      bullet.adapter.add("hidden", function (hidden, target) {
        const index = target.dataItem?.index;
        const total = lineChart.data.length;
        return index !== total - 1 && index % 10000 !== 0;
      });

      series.tooltipText = "{name}: {valueY.formatNumber('#.0')}";
    }

    createSeries("heart", "Heart Rate", "#FF0000");
    createSeries("oxygen", "Oxygen Level", "#008000");
    createSeries("temp", "Body Temp", "#0000FF");

    lineChart.cursor = new am4charts.XYCursor();
    lineChart.legend = new am4charts.Legend();

    setInterval(() => {
      let time = new Date();
      lineChart.addData(
        {
          date: time,
          heart: 72 + Math.random() * 1.5,
          oxygen: 97 + Math.random() * 1.5,
          temp: 36.5 + Math.random() * 0.2,
        },
        1
      );

      let rangeStart = new Date(time.getTime() - 2000);
      dateAxis.zoomToDates(rangeStart, time, false, true);
    }, 200);

    // --- PIE CHART 3D ---
    var pieChart = am4core.create("chartdiv", am4charts.PieChart3D);
    pieChart.hiddenState.properties.opacity = 0;

    pieChart.data = [
      { category: "Heart Rate", value: 72, color: "#FF0000" },
      { category: "Oxygen", value: 98, color: "#008000" },
      { category: "Body Temp", value: 36.6, color: "#0000FF" },
    ];

    pieChart.depth = 20;
    pieChart.angle = 15;

    var pieSeries = pieChart.series.push(new am4charts.PieSeries3D());
    pieSeries.dataFields.value = "value";
    pieSeries.dataFields.category = "category";
    pieSeries.slices.template.propertyFields.fill = "color";
    pieSeries.labels.template.text = "{category}\n{value}";
    pieSeries.slices.template.tooltipText = "{category}: [bold]{value}[/]";

    pieChart.legend = new am4charts.Legend();
  });
});
async function getMistralInsight() {
  const prompt = "Check the vital signs of the patients and give insights and recommendations (Always make sure only basic recommendations because you're still not a doctor itself).";

  const response = await fetch("https://api.mistral.ai/v1/chat/completions", {
    method: "POST",
    headers: {
      "Authorization": "Bearer YOUR_API_KEY", // Replace this
      "Content-Type": "application/json"
    },
    body: JSON.stringify({
      model: "mistral-tiny",
      messages: [{ role: "user", content: prompt }]
    })
  });

  const data = await response.json();
  document.getElementById("ai-response").textContent =
    data.choices?.[0]?.message?.content || "No response.";
}

// Run this when the page loads
getMistralInsight();
