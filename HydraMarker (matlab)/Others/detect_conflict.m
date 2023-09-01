function has_conflict = detect_conflict(state,shape)
    
    [M,N] = size(state.val);
    has_conflict = 0;
    
    for shape_it = 1 : size(shape.val,1)
        shape_cur = shape.val{shape_it};
        sample_num = sum(shape_cur,'all');
        [m,n] = find(shape_cur);
        
        field_stack = zeros(M*N*4,sample_num);
        for rot = 0 : 3
            field = rot90(state.val,rot);
            field_slice = zeros(size(field,1),size(field,2),sample_num);
            for it = 1 : length(m)
            	field_slice(:,:,it) = imtranslate(field,[1-n(it),1-m(it)],'FillValues',nan);
            end
            field_stack(M*N*rot+1:M*N*(rot+1),:) = reshape(field_slice,[M*N,sample_num]);
        end % for rot
        
        field_stack(any(field_stack==-2|isnan(field_stack),2),:) = [];
        field_stack(field_stack==-1) = NaN; % treat unknowns as distinct
        field_stack_unique = unique(field_stack,'stable','rows');
        if size(field_stack_unique,1)<size(field_stack,1) % has conflict is repeating tags occur
            has_conflict = 1;
            return;
        end
    end % for shape
end

