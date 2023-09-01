% Organize grids based on point list "ptList" and ledge information "ledge"

% - array
%   Nx1 cell, each one corresponds to a connected grid, representing a
%   point grid based on their indexes in "ptList" (unavailable elements are NaN).

function [array,E] = ptStruct(ptList,ledge)

    if size(ptList,1)<3
        array = [];
        E = [];
        return;
    end

    % triangulation, remove hypotenuses, merged into squares
    % (the edges of squares should be consist with white/black jump angle)
    pts = fliplr(ptList);
    cnt = delaunay(pts);

    E = edges(triangulation(cnt,pts));
    E_unmatched = false(size(E,1),1);
    for e = 1 : size(E,1)
        ledge_A = deg2rad(ledge(E(e,1),:));
        ledge_B = deg2rad(ledge(E(e,2),:));
        
        ptA = ptList(E(e,1),:);
        ptB = ptList(E(e,2),:);    
        vec = ptB-ptA;
        dirA = atan2(vec(:,1),vec(:,2));
        dirB = atan2(-vec(:,1),-vec(:,2));
            
        ledge_diff = min(abs(angdiff(ledge_A(1),ledge_B(2))),...
                         abs(angdiff(ledge_A(2),ledge_B(1))));
        ledge_A_diff = min([abs(angdiff(dirA,ledge_A(1))),...
                            abs(angdiff(dirA,ledge_A(1)+pi)),...
                            abs(angdiff(dirA,ledge_A(2))),...
                            abs(angdiff(dirA,ledge_A(2)+pi))]);
        ledge_B_diff = min([abs(angdiff(dirB,ledge_B(1))),...
                            abs(angdiff(dirB,ledge_B(1)+pi)),...
                            abs(angdiff(dirB,ledge_B(2))),...
                            abs(angdiff(dirB,ledge_B(2)+pi))]);
        % the included angle should not be larger than 15бу
        min_rad = deg2rad(15);
        if ledge_diff > min_rad || ledge_A_diff > min_rad || ledge_B_diff >min_rad
           E_unmatched(e) = true;
        end
    end
    E(E_unmatched,:) = [];
    
    % remove overlong edges
    E(:,3) = vecnorm(pts(E(:,1),:)-pts(E(:,2),:),2,2);
    E = sortrows(E,3);
    E(:,4) = realmax;
    E(:,5) = ones(size(E,1),1);
    for it = 1 : size(E,1)
        if E(it,3) > 1.5*E(it,4)
            % long edge should not be the 1.5x (or more) of short edge
            E(it,5) = 0;
            continue;
        end
        idxA = E(it,1);
        idxB = E(it,2);
        rowIdx = (E(:,1)==idxA|E(:,1)==idxB|E(:,2)==idxA|E(:,2)==idxB);
        E(rowIdx,4) = min(E(it,3),E(rowIdx,4));
    end
    E(E(:,5)==0,:)=[];
    E(:,3:end) = [];
    
    if isempty(E)
        array = [];
        return;
    end
    % propagation the compass to organize grids
    E_bi = sortrows([E;fliplr(E)]);
    
    compass = sort(ledge,2);
    compass = [compass,compass+180];
    
    matrixLabel = zeros(size(ledge,1),3);
    matrixNum = 1;
    
    queue = E_bi(1); 
    
    % the unset queue of "matrixLabel", which varies with the prpagating of
    % compass
    matrixLabel(queue(1),:) = [1,1,1];
    
    shift = [-1 0;0 1;1 0;0 -1];
    
    finished = false;
    while ~finished
        if ~isempty(queue)
            quA = queue(1);
            quB = E_bi(E_bi(:,1)==quA,2);
            quB(matrixLabel(quB,1)~=0) = [];
            if isempty(quB)
                queue(1) = [];
                continue;
            end
            
            queue = [queue;quB];
            
            ptA = ptList(quA,:);
            ptB = ptList(quB,:);
            
            vec = ptB-ptA;
            dirA = atan2(vec(:,1),vec(:,2));
            dirB = atan2(-vec(:,1),-vec(:,2));
            includeA = abs(angdiff(repmat(dirA,[1,4]),repmat(deg2rad(compass(quA,:)),[size(dirA,1),1])));
            includeB = abs(angdiff(repmat(dirB,[1,4]),deg2rad(compass(quB,:))));
            
            for it = 1 : size(quB,1)
                [~,matched_dirA] = min(includeA(it,:));
                matrixLabel(quB(it),1) = matrixLabel(quA,1);
                matrixLabel(quB(it),2:3) = matrixLabel(quA,2:3) + shift(matched_dirA,:);
                
                expected_matched_dirB = rem(matched_dirA+1,4)+1;
                [~,matched_dirB] = min(includeB(it,:));
                compass(quB(it),:) = circshift(compass(quB(it),:),expected_matched_dirB-matched_dirB);
            end
            queue(1) = [];
        else
            unset = find(matrixLabel(:,1)==0);
            if isempty(unset)
                finished = true;
                continue;
            end
            queue(1) = unset(1);
            matrixNum = matrixNum + 1;
            matrixLabel(queue(1),:) = [matrixNum,1,1];
        end % if ~isempty(queue)
    end % while ~finished
    
    % record the grids into "array"
    array = cell(matrixNum,1);
    matrixLabel(:,4) = 1 : size(matrixLabel,1);
    for num = 1 : matrixNum
        label = matrixLabel(matrixLabel(:,1)==num,:);
        label(:,1) = [];
        label(:,1) = label(:,1) - min(label(:,1)) + 1;
        label(:,2) = label(:,2) - min(label(:,2)) + 1;
        
        M = max(label(:,1));
        N = max(label(:,2));
        array{num} = nan(M,N);
        ind = sub2ind([M,N],label(:,1),label(:,2));
        
        array{num}(ind) = label(:,3);
    end

end

