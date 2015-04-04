    <link href="layout.css" rel="stylesheet" type="text/css"></link>
    <!--[if IE]><script language="javascript" type="text/javascript" src="../excanvas.pack.js"></script><![endif]-->
    <script language="javascript" type="text/javascript" src="../jquery.js"></script>
    <script language="javascript" type="text/javascript" src="../jquery.flot.js"></script>

    <h1>Flot Examples</h1>

    <div id="placeholder" style="width:600px;height:300px;"></div>

    <div id="overview" style="margin-left:50px;margin-top:20px;width:400px;height:50px"></div>

<script id="source" language="javascript" type="text/javascript">
$(function () {
    var d = [[1196463600000, 0], [1196550000000, 0], [1196636400000, 0], [1196722800000, 77], [1196809200000, 3636], [1196895600000, 3575], [1196982000000, 2736], [1197068400000, 1086], [1197154800000, 676], [1197241200000, 1205], [1197327600000, 906], [1197414000000, 710], [1197500400000, 639], [1197586800000, 540], [1197673200000, 435], [1197759600000, 301], [1197846000000, 575], [1197932400000, 481], [1198018800000, 591], [1198105200000, 608], [1198191600000, 459], [1198278000000, 234], [1198364400000, 1352], [1198450800000, 686], [1198537200000, 279], [1198623600000, 449], [1198710000000, 468], [1198796400000, 392], [1198882800000, 282], [1198969200000, 208], [1199055600000, 229], [1199142000000, 177], [1199228400000, 374], [1199314800000, 436], [1199401200000, 404], [1199487600000, 253], [1199574000000, 218], [1199660400000, 476], [1199746800000, 462], [1199833200000, 448], [1199919600000, 442], [1200006000000, 403], [1200092400000, 204], [1200178800000, 194], [1200265200000, 327], [1200351600000, 374], [1200438000000, 507], [1200524400000, 546], [1200610800000, 482], [1200697200000, 283], [1200783600000, 221], [1200870000000, 483], [1200956400000, 523], [1201042800000, 528], [1201129200000, 483], [1201215600000, 452], [1201302000000, 270], [1201388400000, 222], [1201474800000, 439], [1201561200000, 559], [1201647600000, 521], [1201734000000, 477], [1201820400000, 442], [1201906800000, 252], [1201993200000, 236], [1202079600000, 525], [1202166000000, 477], [1202252400000, 386], [1202338800000, 409], [1202425200000, 408], [1202511600000, 237], [1202598000000, 193], [1202684400000, 357], [1202770800000, 414], [1202857200000, 393], [1202943600000, 353], [1203030000000, 364], [1203116400000, 215], [1203202800000, 214], [1203289200000, 356], [1203375600000, 399], [1203462000000, 334], [1203548400000, 348], [1203634800000, 243], [1203721200000, 126], [1203807600000, 157], [1203894000000, 288]];

    // helper for returning the weekends in a period
    function weekendAreas(plotarea) {
        var areas = [];
        var d = new Date(plotarea.xmin);
        // go to the first Saturday
        d.setDate(d.getDate() - ((d.getDay() + 1) % 7))
        d.setSeconds(0);
        d.setMinutes(0);
        d.setHours(0);
        var i = d.getTime();
        do {
            // when we don't set y1 and y2 the rectangle
            // automatically extends to infinity in those directions
            areas.push({ x1: i, x2: i + 2 * 24 * 60 * 60 * 1000 });
            i += 7 * 24 * 60 * 60 * 1000;
        } while (i < plotarea.xmax);

        return areas;
    }
    
    var options = {
        xaxis: { mode: "time" },
        selection: { mode: "x" },
        grid: { coloredAreas: weekendAreas }
    };
    
    var plot = $.plot($("#placeholder"), [d], options);
    
    var overview = $.plot($("#overview"), [d], {
        lines: { show: true, lineWidth: 1 },
        shadowSize: 0,
        xaxis: { ticks: [], mode: "time" },
        yaxis: { ticks: [], min: 0, max: 4000 },
        selection: { mode: "x" }
    });

    // now connect the two
    var internalSelection = false;
    
    $("#placeholder").bind("selected", function (event, area) {
        // do the zooming
        plot = $.plot($("#placeholder"), [d],
                      $.extend(true, {}, options, {
                          xaxis: { min: area.x1, max: area.x2 }
                      }));
        
        if (internalSelection)
            return; // prevent eternal loop
        internalSelection = true;
        overview.setSelection(area);
        internalSelection = false;
    });
    
    $("#overview").bind("selected", function (event, area) {
        if (internalSelection)
            return;
        internalSelection = true;
        plot.setSelection(area);
        internalSelection = false;
    });
});
</script>

 </body>
</html>