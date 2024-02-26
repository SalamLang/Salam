<?php
$age = 10;
echo $age;
{
    echo $age;
    {
        $age = 20;
        echo $age;
    }
    echo $age;
    {
        $age = $age + 5;
        echo $age;
    }
    echo $age;
}
echo $age;
