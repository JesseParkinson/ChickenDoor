/**
 * @file builtinfiles.h
 * @brief This file is part of the WebServer example for the ESP8266WebServer.
 *  
 * This file contains long, multiline text variables for  all builtin resources.
 */

// used for $upload.htm
static const char uploadContent[] PROGMEM =
R"==(
<!doctype html>
<html lang='en'>

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Upload</title>
</head>

<body style="width:300px">
  <h1>Upload</h1>
  <div><a href="/">Home</a></div>
  <hr>
  <div id='zone' style='width:16em;height:12em;padding:10px;background-color:#ddd'>Drop files here...</div>

  <script>
    // allow drag&drop of file objects 
    function dragHelper(e) {
      e.stopPropagation();
      e.preventDefault();
    }

    // allow drag&drop of file objects 
    function dropped(e) {
      dragHelper(e);
      var fls = e.dataTransfer.files;
      var formData = new FormData();
      for (var i = 0; i < fls.length; i++) {
        formData.append('file', fls[i], '/' + fls[i].name);
      }
      fetch('/', { method: 'POST', body: formData }).then(function () {
        window.alert('done.');
      });
    }
    var z = document.getElementById('zone');
    z.addEventListener('dragenter', dragHelper, false);
    z.addEventListener('dragover', dragHelper, false);
    z.addEventListener('drop', dropped, false);
  </script>
</body>
)==";

// used for $upload.htm
static const char notFoundContent[] PROGMEM = R"==(
<html>
<head>
  <title>Resource not found</title>
</head>
<body>
  <p>Knucklehead! The resource was not found.</p>
  <p><a href="/">Start again</a></p>
  <p>The time is 
  <!-- #BeginDate format:Am1a -->epochTime<!-- #EndDate -->
</p>
</body>
)==";

// used for Manual Open
static const char manualOpen[] PROGMEM = R"==(
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Manual and Open</title>
</head>

<body>
<h1>Chicken Door Control</h1>
<p>The door is currently open and in manual mode.</p>

<form id="form1" name="form1" method="post" action="">
  Switch to Auto
  <input type="submit" name="GoToAuto" id="GoToAuto" value="Submit" />
</form>
<br />
<form id="form2" name="form2" method="post" action="">
  Close the door
  <input type="submit" name="CloseDoor" id="CloseDoor" value="Submit" />
</form>
<p>The time is 
  <!-- #BeginDate format:Am1a -->September 6, 2023 4:35 AM<!-- #EndDate -->
</p>
</body>
</html>
)==";

// use for Manual Closed
static const char manualClosed[] PROGMEM = R"==(
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Manual and Closed</title>
</head>

<body>
<h1>Chicken Door Control</h1>
<p>The door is currently closed and in manual mode.</p>

<form id="form1" name="form1" method="post" action="">
  Switch to Auto
  <input type="submit" name="GoToAuto" id="GoToAuto" value="Submit" />
</form>
<br />
<form id="form2" name="form2" method="post" action="">
  Open the door
    <input type="submit" name="OpenDoor" id="OpenDoor" value="Submit" />
</form>
<p>The time is 
  <!-- #BeginDate format:Am1a -->September 6, 2023 4:35 AM<!-- #EndDate -->
</p>
</body>
</html>
)==";

  // use for Auto Open
  static const char autoOpen[] PROGMEM = R"==(
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Automatic and Open</title>
</head>

<body>
<h1>Chicken Door Control</h1>
<p>The door is currently open and in automatic mode.</p>

<form id="form1" name="form1" method="post" action="">
  Switch to Manual
  <input type="submit" name="GoToManual" id="GoToManual" value="Submit" />
</form>
<br />
<form id="form2" name="form2" method="post" action="">
  Close the door
  <input type="submit" name="CloseDoor" id="CloseDoor" value="Submit" />
</form>
<p>The time is 
  <!-- #BeginDate format:Am1a -->September 6, 2023 4:35 AM<!-- #EndDate -->
</p>
</body>
</html>
)==";

//use for Auto Closed
static const char autoClosed[] PROGMEM = R"==(
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Automatic and Closed</title>
</head>

<body>
<h1>Chicken Door Control</h1>
<p>The door is currently closed and in automatic mode.</p>

<form id="form1" name="form1" method="post" action="">
  Switch to Manual
  <input type="submit" name="GoToManual" id="GoToManual" value="Submit" />
</form>
<br />
<form id="form2" name="form2" method="post" action="">
  Open the door
  <input type="submit" name="OpenDoor" id="OpenDoor" value="Submit" />
</form>
<p>The time is 
  <!-- #BeginDate format:Am1a -->September 6, 2023 4:35 AM<!-- #EndDate -->
</p>
</body>
</html>

)==";
