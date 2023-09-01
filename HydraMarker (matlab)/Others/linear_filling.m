function [state,process_record] = linear_filling(state,shape,max_time,max_iter)
    unum = sum(state.val==-1,'all');
    tic;
    process_record = [];
    tree = [1,1];
    iter = 1;
    it = 0;
    while 1
        it = it + 1;
        if tree(iter,1)==1
            state.val(iter)=0;
            tree(iter,1)=0;
        elseif tree(iter,2)==1
            state.val(iter)=1;
            tree(iter,2)=0;
        else
            state.val(iter) = -1;
            iter = iter - 1;
            continue;
        end
        iter = iter+1;
        tree(iter,:) = [1,1];
        
        if ~any(state.val==-1,'all')
            break;
        end
        if detect_conflict(state,shape)
           state.val(iter) = -1;
           iter = iter - 1;
           continue;
        end
        
        imshow(Cpainter.draw_bw(state,5));
        fprintf('\n step:%d/%d time:%0.1fs/%0.1fs tree_depth:%d/%d',it,max_iter,toc,max_time,iter-1,unum);
        pause(0.001);
        process_record = [process_record;it,toc,iter];
        if toc>max_time || it>max_iter
            fprintf('\n time/iter out!\n');
            state.val = [];
            return;
        end
        
    end
end

