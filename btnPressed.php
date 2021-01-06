<?php
 if (isset($_POST["test"]))
   {
       $a= $_POST["test"];
      
       echo $a;
   }
   if (isset($_POST["cardNum"]))
   {

       $b = $_POST["cardNum"];
       echo $b;
   }

   $c = exec("/opt/lampp/htdocs/one $a $b");
?>
