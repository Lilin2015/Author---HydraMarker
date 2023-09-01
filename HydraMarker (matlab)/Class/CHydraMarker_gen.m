classdef CHydraMarker_gen < handle

    properties (SetAccess=private)
        state Dfield
        shape Dshape
        record
        process_record
        unum
    end
    
    properties (Access=private)
        state_mngr Cstate_mngr
        procs_mngr Cprocs_mngr
        max_time
        max_iter
    end
    
    methods

        function assign(obj,state_raw,shape,max_time,max_iter)
            %mustBeA(state_raw,'Dfield');
            %mustBeA(shape,'Dshape');
            
            obj.unum = sum(state_raw.val==-1,'all');

            mustBeInteger(max_iter);
            assert(max_iter>0,'max_iter must be positive!');
            
            obj.state_mngr = Cstate_mngr();
            obj.procs_mngr = Cprocs_mngr();
            
            obj.state = state_raw;
            obj.shape = shape;
            obj.state_mngr.assign(shape);
            obj.procs_mngr.assign(obj.unum,max_iter);
            obj.max_time = max_time;
            obj.max_iter = max_iter;
        end
         
        function generate(obj,flag)
            if ~exist('flag','var')
                flag = 's-first';
            end
            tic;
            obj.process_record = [];
            Length=0;
            while 1
                [msg,s_state] = obj.state_mngr.update(obj.state);                
                [msg,sugst] = obj.procs_mngr.procs_sugst(msg,[]);
                if strcmp(msg.str,'modify')
                    obj.state.val(sugst.ind) = sugst.val;
                    continue;
                end
                [msg,sugst] = obj.state_mngr.filling_sugst(s_state,flag);
                [msg,sugst] = obj.procs_mngr.procs_sugst(msg,sugst);
                if strcmp(msg.str,'modify')
                    obj.state.val(sugst.ind) = sugst.val;
                elseif strcmp(msg.str,'procs_complete')
                    break;
                end

                imshow(Cpainter.draw_bw(obj.state,5));
                valid_num = valid_tag_num(obj.state,obj.shape);
                fprintf(repmat('\b', 1, Length));
                Length = fprintf('\n bWFC-sf, step:%d/%d time:%0.1fs/%0.1fs valid_tag:%d',obj.procs_mngr.iter,obj.max_iter,toc,obj.max_time,valid_num);
                obj.process_record = [obj.process_record;obj.procs_mngr.iter,toc,valid_num];
                if toc>obj.max_time || obj.procs_mngr.is_end()
                    fprintf('--->time/iter out!');
                    obj.state.val = [];
                    return;
                end
                pause(0.0001);
            end % end while
            obj.record = obj.procs_mngr.full_record;
            obj.record(any(isnan(obj.record),2),:)=[];
        end % end function
    end % end methods
    
end

