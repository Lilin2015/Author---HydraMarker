function [state,process_record] = DOF_filling(state,shape,max_time,max_iter)
    tic;
    process_record = [];
    iter = 1;
    tree = [1,1,1];
    it = 0;
    Length = 0;
    while 1
        it = it + 1;
        if tree(iter,2)==1
            state.val(tree(iter,1))=0;
            tree(iter,2)=0;
        elseif tree(iter,3)==1
            state.val(tree(iter,1))=1;
            tree(iter,3)=0;
        else
            state.val(tree(iter,1)) = -1;
            iter = iter - 1;
            continue;
        end
        safty = safty_freedom(state,shape);
        [~,idx] = min(safty,[],'all','linear');
        iter = iter+1;
        tree(iter,:) = [idx,1,1];
            
        if ~any(state.val==-1,'all')
            break;
        end
        if detect_conflict(state,shape)
           state.val(tree(iter,1)) = -1;
           iter = iter - 1;
           continue;
        end
        
        imshow(Cpainter.draw_bw(state,5));
        valid_num = valid_tag_num(state,shape);
        fprintf(repmat('\b', 1, Length));
        Length = fprintf('\nDOF-filling, step:%d/%d time:%0.1fs/%0.1fs valid_tag:%d',it,max_iter,toc,max_time,valid_num);
        pause(0.001);
        process_record = [process_record;it,toc,valid_num];
        if toc>max_time || it>max_iter
            fprintf('--->time/iter out!');
            state.val = [];
            return;
        end
    end
end

