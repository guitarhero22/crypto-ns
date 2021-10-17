wd=`pwd`
echo $wd

mkdir build
cd build
cmake .. && cmake --build .

mkdir results
cp crypto-ns results
cd results

for adversary in selfish stubborn
do
    echo $adversary >> results.txt
    for compute_power in 20 22.5 25 27.5 30 32.5 35 37.5 40 42.5 45
    do
        for num_con in 25 50 75
        do
            dir="$adversary"_"$compute_power"_"$num_con"
            echo $dir
            mkdir "$dir"

            cd $dir
            mkdir dumps dots
            python3 "$wd"/config2/config_script.py --num_con $num_con --compute $compute_power --adversary $adversary > conf 
            cat conf | ../crypto-ns >> ../results.txt
            cd ..
        done
    done
done