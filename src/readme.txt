// PATRAS ANTON-FABIAN
// 324CB
// APRILIE 2020

Salutari,
	"Protocolul" implementat de catre mine care vine deasupra TCP-ului este
compus din structurile TPkg si TSmall care au ca prim camp un uint8_t prin care
pot identifica ce pachet a fost primit. Astfel pot realiza comunicare 
client TCP <-> server prin aceste pachete. TPkg este pentru informatiile "mici"
gen log-in, log-out, subscribe, unsubscribe (dinspre client spre server) sau
shutdown order (dinspre server spre client). TSmall este pentru transmiterea
datelor parsate de server catre clientii TCP.

	Serverul este o structura ce are ca si cele mai importante campuri doua
liste. O lista este de subscriberi, iar cea de-a doua lista este de topicuri.
Subscriberii din lista de subscriberi sun reprezentati cu ajutorul unei
structuri TSubscriber care contine informatii: ID, fd, lista de topicuri la
care este abonat.

	Lista de topicuri (din server) contine structuri de tip TTopic care pot 
contine liste de perechi (subcriber, SF). Aceste perechi sunt utilizate in
structurile de tip TTopic existente in lista de topicuri din server.

	Am ales sa mentin informatia subscriber <-> topic abonat in doua locuri
pentru a imi fi mai "usor" (de fapt, mai rapid) sa pot verifica cine este 
subscribed unde si ce topic ce subcriberi are. Astfel, daca doresc sa trimit
un anumit mesaj tuturor utilizatorilor abonatio unui anumit topic stiu ca 
acestia vor fi toti in lista de perechi (abonat, SF) une stiu si daca trebuie
sa retin informatia daca subscriberul este offline. Daca doresc sa verific 
un utiliztor unde este abonat ca sa il pot dezabona sau sa-l abonez pot
interoga lista lui de topicuri neavand nevoie sa verific daca este "prezent"
in lista de subscriberi al fiecarui topic din lista de topicuri al serverului.

	Am cam facut error handling la inputul de al stdin ca sa nu ajuns nici
la client, dar mai ales nici la server intr-o situatie in care sa crape
vreunul dintre ei din cauza inputului. Functionalitatile cerute in tema
functioneaza si am avut grija sa eliberez memoria. Acum nu stiu cum o sa
fie testata tema deci .. cine stie ce poate aparea.