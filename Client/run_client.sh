# Progetto IIW - Script di avvio sequenziale per installazione e demo
# Versione: 11.0
# Autore: Alessio Pompeo
# Autore: Davide Edoardo Stoica
# Autore: Consuelo Caporaso
# Anno: 2025

make clean
make
echo " "
echo " "
echo " "
echo " "
echo " "
echo " "
echo "----------------------------------------------------------"
echo "--------------------Startup Client-------------------------"
echo "----------------------------------------------------------"
echo " "
echo " "
#valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind_output.txt ./my_client
./my_client
