function state = method_conflict_rectify(state,shape,max_iter)
    unknowns = find(state.val==-1);
    state.val(unknowns)=double(rand(size(unknowns))>0.5);
    %% find and reassign conflict regions
    for it = 1 : max_iter
        conflict = find_conflict(state,shape);
        if all(conflict==0,'all')
            return;
        end
        subplot(2,2,1); imshow(Cpainter.draw_bw(state,40));
        subplot(2,2,2); imshow(imresize(conflict,40,'nearest'),[]);
        
        %rectify_pos = (conflict==max(conflict,[],'all'));
        rectify_pos = conflict>=median(conflict,'all');
        state.val(rectify_pos)=-1;
        subplot(2,2,3); imshow(Cpainter.draw_bw(state,40));
        % re-assign conflicts
        state.val(rectify_pos)=(rand(sum(rectify_pos,'all'),1)>0.5);
        subplot(2,2,4); imshow(Cpainter.draw_bw(state,40));
        
        pause(0.3);
    end
    
end

