document.addEventListener("DOMContentLoaded", function () {
  am4core.ready(function () {
    am4core.useTheme(am4themes_animated);

    // LINE CHART
    var lineChart = am4core.create("lineChart", am4charts.XYChart);
    lineChart.paddingRight = 20;

    lineChart.data = [
      { date: "2025-05-01", heart: 72, oxygen: 98, temp: 36.6 },
      { date: "2025-05-02", heart: 75, oxygen: 97, temp: 36.5 },
      { date: "2025-05-03", heart: 73, oxygen: 99, temp: 36.7 },
      { date: "2025-05-04", heart: 76, oxygen: 96, temp: 36.8 }
    ];

    var dateAxis = lineChart.xAxes.push(new am4charts.DateAxis());
    dateAxis.renderer.grid.template.location = 0;

    var valueAxis = lineChart.yAxes.push(new am4charts.ValueAxis());

    function createLineSeries(field, name, color) {
      var series = lineChart.series.push(new am4charts.LineSeries());
      series.dataFields.valueY = field;
      series.dataFields.dateX = "date";
      series.name = name;
      series.stroke = am4core.color(color);
      series.strokeWidth = 2;
      series.bullets.push(new am4charts.CircleBullet());
      series.tooltipText = "{name}: {valueY}";
    }

    createLineSeries("heart", "Heart Rate", "#FF0000");
    createLineSeries("oxygen", "Oxygen Level", "#008000");
    createLineSeries("temp", "Body Temp", "#0000FF");

    lineChart.legend = new am4charts.Legend();
    lineChart.cursor = new am4charts.XYCursor();

    // PIE CHART
    var chart = am4core.create("chartdiv", am4charts.PieChart3D);
    chart.hiddenState.properties.opacity = 0;

    chart.data = [
      { category: "Heart Rate", value: 72, color: "#FF0000" },
      { category: "Oxygen", value: 98, color: "#008000" },
      { category: "Body Temp", value: 36.6, color: "#0000FF" },
      { category: "Alerts", value: 2, color: "#FFA500" }
    ];

    chart.depth = 20;
    chart.angle = 15;

    var pieSeries = chart.series.push(new am4charts.PieSeries3D());
    pieSeries.dataFields.value = "value";
    pieSeries.dataFields.category = "category";
    pieSeries.slices.template.propertyFields.fill = "color";
    pieSeries.labels.template.text = "{category}\n{value}";
    pieSeries.slices.template.tooltipText = "{category}: [bold]{value}[/]";

    chart.legend = new am4charts.Legend();
  });
});
