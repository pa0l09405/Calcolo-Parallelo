%% FASE ACQUISIZIONE DATI RELATIVI ALLE PRESTAZIONI

warning('OFF', 'MATLAB:table:ModifiedAndSavedVarnames');
p=[2,4,8];
tabella_tempi = readtable('test_valutazione_integrale.xlsx', 'Range', 'B3:G6','ReadRowName', true);
tempi_img = imread('tempi.png');
figure(1)
imshow(tempi_img)
title('Tempi')

%% ANALISI PRESTAZIONI TEMPI

c = [table2array(tabella_tempi(:,1)),table2array(tabella_tempi(:,2)),table2array(tabella_tempi(:,3)),table2array(tabella_tempi(:,4)),table2array(tabella_tempi(:,5))];
figure(2)
plot(p,c(:,1),'-*',p,c(:,2),'-o',p,c(:,3),'-+',p,c(:,4),'-gs',p,c(:,5),'-md','LineWidth',2)
title('Analisi tempi')
legend('N=10^4','N=10^5','N=10^6','N=10^7','N=10^8')
xlim([p(1) p(3)])
xlabel('Numero di thread');
ylabel('Tempo di esecuzione (secondi)');
