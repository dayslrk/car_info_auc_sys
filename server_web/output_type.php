<!DOCTYPE html>
<html>
<head>

		<meta charset="UTF-8">

		<title>配件信息</title>

	</head>

	<style type="text/css">

		body {

			background-image: url(student.jpg);

			background-size: 100%;

		}



		.wrapper {

			width: 1000px;

			margin: 20px auto;

		}



		h1 {

			text-align: center;

		}



		.add {

			margin-bottom: 20px;

		}



		.add a {

			text-decoration: none;

			color: #fff;

			background-color: #00CCFF;

			padding: 6px;

			border-radius: 5px;

		}



		td {

			text-align: center;

		}

	</style>
<body>
<table width="960" border='1'>
         <tr><th>类型ID</th><th>类型名</th><th>零件列表ID</th></tr>
    <?php
    
        include 'conn.php';
        //查询数据表中的数据
         $sql = mysqli_query($link,"select * from type");
         $datarow = mysqli_num_rows($sql); //长度
            //循环遍历出数据表中的数据
            for($i=0;$i<$datarow;$i++){
                $sql_arr = mysqli_fetch_assoc($sql);
                $id = $sql_arr[Type_ID];
                $name = $sql_arr[Type_Name];
                $list_id = $sql_arr[DevList_ID];
               
                echo "<tr><td>$id</td><td>$name</td><td>$list_id</td></tr>";
            }
    ?>
</table>
</body>
</html>
