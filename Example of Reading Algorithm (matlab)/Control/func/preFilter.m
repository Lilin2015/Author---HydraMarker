% Read feature candidates from "img" and output them in "ptList".

% - ptList
%   Nx2 matrix, each row corresponds to the row-column coordinates of a
%   feature candidate.

% - img
%   Input image must be single channel, double type.

% - r
%   The area of a cross point is a square with side-length "2xr+1".
%   The points near boundaries are rejected based on this value.

% - expectN
%   The upper bound of the number of candidates.

% - sigma
%   the propagating range in cross point detection.

function ptList = preFilter(img,r,expectN,sigma)

    % "G" is the power of the cancelled out gradients, which is large near
    % high contrast and symmetric patterns (cross point).
    [Gx,Gy] = imgradientxy(img);
    Gpow = imgaussfilt((Gx.^2+Gy.^2).^0.5,sigma);
    Gsum = (imgaussfilt(Gx,sigma).^2 + imgaussfilt(Gy,sigma).^2).^0.5;
    G = Gpow-Gsum;
    
    % Non-maximum suppression    
    G(imdilate(G,strel('square',3))~=G)=0;

    % Pick the candidates with top-"expectN" "G" value
    G(1:r,:) = 0; G(end-r+1:end,:) = 0;
    G(:,1:r) = 0; G(:,end-r+1:end) = 0;
    G_sort = G(:); G_sort(G_sort<0.1)=[];
    G_sort = sort(G_sort,'descend');
    [im,in] = ind2sub(size(G),find(G>=G_sort(min(expectN,size(G_sort,1)))));
    ptList = [im,in];

end