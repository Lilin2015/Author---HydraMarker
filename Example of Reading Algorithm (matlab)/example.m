reset_toolbox;
close all

%% load a marker field
load 10x10_for_3x3_6x2_10x1.mat % this is a 10x10 marker field with 3 tag shapes, 3x3, 6x2, and 10x1

%% read an image containing HydraMarker (choose one)
img = im2double(rgb2gray(imread('10x10_for_3x3_6x2_10x1.bmp'))); % this is a systhetic image
img = im2double(rgb2gray(imread('marker_scroll.jpg')));          % this is a real world image
img = im2double(rgb2gray(imread('marker_tracking.jpg')));        % this is a tracking task
img = imresize(img,720/max(size(img,1),size(img,2)));

%% identify the features of HydraMarker
expectN = 20*(size(sta,1)+1)*(size(sta,2)+1);
[ptList,edge] = read_marker(img,sta,5,expectN,3);

%% display
figure;
imshow(img);
hold on;

% draw dots
scatter(ptList(:,2),ptList(:,1),20,'g','filled','o','LineWidth',1);

% draw edges
Y = ptList(:,1);
X = ptList(:,2);
plot(X(edge'),Y(edge'),'LineWidth',1,'Color','g');

% draw unsure IDs
pt_uID = ptList(isnan(ptList(:,3)),:);
scatter(pt_uID(:,2),pt_uID(:,1),50,'r','x','LineWidth',3);

% draw IDs
pt_ID = ptList(~isnan(ptList(:,3)),:);
text(pt_ID(:,2),pt_ID(:,1),num2str(pt_ID(:,3)),'FontSize',10,'Color','y');
