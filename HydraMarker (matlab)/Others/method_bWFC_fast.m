function [state,process_record] = method_bWFC_fast(state,shape,max_time,max_iter)
    
    %% initial
    unum = sum(state.val==-1,'all');
    procs_mngr = Cprocs_mngr();
    state_mngr = Cstate_mngr();
    procs_mngr.assign(unum,max_iter);
    
    tic;
    process_record = [];
    %% filling
    Length = 0;
    while 1
        
        if detect_conflict(state,shape)
            s_state = [];
            msg = Dmsg('tag_repeated');
        else
            s_state = satfy_to_superstate(shape,state);
            if any(s_state.likelihood)
            end
            msg = Dmsg('empty');
        end
        
        [msg,sugst] = procs_mngr.procs_sugst(msg,[]);
        if strcmp(msg.str,'modify')
            state.val(sugst.ind) = sugst.val;
            continue;
        end
        [msg,sugst] = state_mngr.filling_sugst(s_state,'s-first');
        [msg,sugst] = procs_mngr.procs_sugst(msg,sugst);
        if strcmp(msg.str,'modify')
                state.val(sugst.ind) = sugst.val;
            elseif strcmp(msg.str,'procs_complete')
                break;
        end
        imshow(Cpainter.draw_bw(state,5));
        valid_num = valid_tag_num(state,shape);
        fprintf(repmat('\b', 1, Length));
        Length = fprintf('\nfast-bWFC, step:%d/%d time:%0.1fs/%0.1fs valid_tag:%d',procs_mngr.iter,max_iter,toc,max_time,valid_num);
        
        process_record = [process_record;procs_mngr.iter,toc,valid_num];
        if toc>max_time || procs_mngr.is_end()
            fprintf('--->time/iter out!');
            state.val = [];
            return;
        end
        pause(0.1);
    end % end while
    
    record = procs_mngr.full_record;
    record(any(isnan(record),2),:)=[];

end

