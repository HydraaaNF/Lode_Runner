import subprocess, time

victoire = """/  __ \                           | |       | |     | | (_)               | |
| /  \/ ___  _ __   __ _ _ __ __ _| |_ _   _| | __ _| |_ _  ___  _ __  ___| |
| |    / _ \| '_ \ / _` | '__/ _` | __| | | | |/ _` | __| |/ _ \| '_ \/ __| |
| \__/\ (_) | | | | (_| | | | (_| | |_| |_| | | (_| | |_| | (_) | | | \__ \_|
 \____/\___/|_| |_|\__, |_|  \__,_|\__|\__,_|_|\__,_|\__|_|\___/|_| |_|___(_)
                    __/ |                                                    
                   |___/ """

dossier = subprocess.check_output(["ls"])
if not "logs" in dossier:
    subprocess.check_output(["mkdir", "logs"])

n_test = input("Nombre de tests:")
niveau = input("Quel niveau voulez-vous tester ?")
success = 0
for i in range(n_test):
    results = subprocess.check_output(["./lode_runner", "-debug", "on", "-encoding", "ascii", "-delay", "0", "level"+str(niveau)+".map"])
    print(i)
    if victoire in str(results):
        success+=1
    else:
        print("ECHEC num "+str(i - success + 1))
        f = open("./logs/"+str(time.time())+":"+str(i)+".log", "wt")
        f.write(results)
        f.close()

print("total", n_test, "victoire", success, "taux", float(success)/float(n_test))