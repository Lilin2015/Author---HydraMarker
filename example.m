reset_toolkit;

%% config
% define your initial marker field, which is a matrix that
%   0, preset value
%   1, preset value
% -1, unknown
% -2, hollow
state = -ones(20);

% define N tag shapes, which is a Nx1 cell
% each cell is a matrix that
%  0, hollow
%  1, solid
shape{1,1} = ones(3,4);
% shape{2,1} = ones(3,6);
% shape{3,1} = ones(2,9);
% shape{4,1} = ones(1,20);

% choose the algorithm
% fast-bWFC (recommend)
% bWFC-sf (precise but too slow, not recommend, see the paper)
method = 'fast-bWFC';

%% generate
result = generate_MF(state,shape,method);

%% display
% marker appearance is independent from marker field
% there are some examples
figure;
imshow(Cpainter.draw_deltille(result.state,20))