function conflict = find_conflict(state,shape)
    
    [M,N] = size(state.val);
    Num = numel(state.val);
    conflict = zeros(M,N);
    
    for shape_it = 1 : size(shape.val,1)
        shape_cur = shape.val{shape_it};
        sample_num = sum(shape_cur,'all');
        [m,n] = find(shape_cur);
        [shape_m,shape_n] = size(shape_cur);

        stack = zeros(M*N*4,sample_num);
        % stack = zeros(M,N,sample_num,4);
        for rot = 0 : 3
            field = rot90(state.val,rot);
            field_slice = zeros(size(field,1),size(field,2),sample_num);
            for it = 1 : length(m)
            	field_slice(:,:,it) = imtranslate(field,[1-n(it),1-m(it)],'FillValues',nan);
            end
            stack(M*N*rot+1:M*N*(rot+1),:) = reshape(field_slice,[M*N,sample_num]);
        end
        stack(stack==-2) = nan; % tags with hollows are not considered
        stack(any(isnan(stack),2),:) = nan; % tags out of boundaries are not considered
        stack(stack==-1) = nan; % -1 are considered as distinct

        [~,~,label] = unique(stack,'stable','rows');
        cnt = histcounts(label,1:size(stack,1)+1);
        [LocA,LocB] = ismember(label,find(cnt>1));
        repeat = cnt(cnt>1)';
        LocB(LocB==0)=[];
        idx = find(LocA);
        idx = [ceil(idx/Num),rem(idx,M*N),repeat(LocB)];
        idx(idx==0)=Num;

        conflict_add = zeros(N,M);
        for it = 1 : 4
            conflict_add = rot90(conflict_add);
            [im,in] = ind2sub(size(conflict_add),idx(idx(:,1)==it,2));
            repeat_deg = idx(idx(:,1)==it,3);
            for ik = 1 : length(im)
                conflict_add(im(ik):im(ik)+shape_m-1,in(ik):in(ik)+shape_n-1) = ...
                    conflict_add(im(ik):im(ik)+shape_m-1,in(ik):in(ik)+shape_n-1) + ...
                        shape_cur*repeat_deg(ik);
            end
        end
        conflict = conflict+rot90(conflict_add);
    end % for shape
end

