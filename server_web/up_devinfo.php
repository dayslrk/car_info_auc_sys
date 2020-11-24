<!doctype>
<html>
<head>
</head>
<body>
<?php
$connect = mysqli_connect("127.0.0.1","root","123456","mysql");
require_once "Classes/PHPExcel/IOFactory.php";
$filename=$_FILES['excel']['name'];
$html = "<table border='1'>";
$objphpexcel = PHPExcel_IOFactory::load($filename);
foreach ($objphpexcel->getWorksheetIterator() as $worksheet)
 {
 		$highestrow = $worksheet->getHighestRow();

 		for($row=1;$row<=$highestrow;$row++)
 		{
 			
 			$html.="<tr>";
			//alter according to no of columns excel has 
 	$col1 = mysqli_real_escape_string($connect,$worksheet->getCellByColumnAndRow(0,$row)->getValue());
 		
 	$col2 = mysqli_real_escape_string($connect,$worksheet->getCellByColumnAndRow(1,$row)->getValue());

 	$col3 = mysqli_real_escape_string($connect,$worksheet->getCellByColumnAndRow(2,$row)->getValue());

 	$col4 = mysqli_real_escape_string($connect,$worksheet->getCellByColumnAndRow(3,$row)->getValue());

     $col5 = mysqli_real_escape_string($connect,$worksheet->getCellByColumnAndRow(4,$row)->getValue());
     $col6 = mysqli_real_escape_string($connect,$worksheet->getCellByColumnAndRow(3,$row)->getValue());

 	$col7 = mysqli_real_escape_string($connect,$worksheet->getCellByColumnAndRow(4,$row)->getValue());

         
		$sql = "INSERT INTO dev(Dev_ID,Dev_Name,Dev_Key,DevList_ID,Dev_Time,DEV_OEM,Status) VALUES ('".$col1."' , '".$col2."','".$col3."' , '".$col4."','".$col5."','".$col6."' , '".$col7."')";
 		mysqli_query($connect,$sql);
 			$html.= '<td>' .$col1. '</td>';
 			$html.= '<td>' .$col2. '</td>';
 			$html.= '<td>' .$col3. '</td>';
 			$html.= '<td>' .$col4. '</td>';
             $html.= '<td>' .$col5. '</td>';
             $html.= '<td>' .$col6. '</td>';
 			$html.= '<td>' .$col7. '</td>';
 			$html.= "<tr>";
 		}


	}

	$html.= '</table>';

	 echo $html; 
?>

</body>
</html>
