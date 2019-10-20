#!/bin/sh

rm -rf test2.db
sqlite3 test2.db "CREATE TABLE info (name TEXT, age INT, salary INT)"
rand(){
  min=$1
  max=$(($2-$min+1))
  num=$(date +%s%N)
  echo $(($num%$max+$min))
}

randname(){
  which=$(rand 0 10)
  #echo $which
  if [ $which -eq 0 ]
  then
    echo \'ruby\'
  elif [ $which -eq 1 ]
  then
    echo \'lily\'
  elif [ $which -eq 2 ]
  then
    echo \'jhon\'
  elif [ $which -eq 3 ]
  then
    echo \'jobs\'
  elif [ $which -eq 4 ]
  then
    echo \'cook\'
  elif [ $which -eq 5 ]
  then
    echo \'sam\'
  elif [ $which -eq 6 ]
  then
    echo \'bill\'
  elif [ $which -eq 7 ]
  then
    echo \'tom\'
  elif [ $which -eq 8 ]
  then
    echo \'jerry\'
  elif [ $which -eq 9 ]
  then
    echo \'hank\'
  else
    echo \'doubi\'
  fi
}

for i in `seq 1 1000`
do
  echo "count:$i"
  name=$(randname)
  echo "name:$name"
  age=$(rand 20 30)
  echo "age:$age"
  salary=`expr $(rand 10 20) \* 1000`
  echo "salary:$salary"
  sqlite3 test.db "INSERT INTO info VALUES ($name, $age, $salary)"
done
