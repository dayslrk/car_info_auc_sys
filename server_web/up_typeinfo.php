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

 	

         
		$sql = "INSERT INTO type(Type_ID,Type_Name,DevList_ID) VALUES ('".$col1."' , '".$col2."','".$col3."')";
 		mysqli_query($connect,$sql);
 			$html.= '<td>' .$col1. '</td>';
 			$html.= '<td>' .$col2. '</td>';
 			$html.= '<td>' .$col3. '</td>';
 			$html.= "<tr>";
 		}


	}

	$html.= '</table>';

	 echo $html; 
?>

</body>
</html>
